#include <iostream>
#include <unordered_map>
#include <deque>
#include <memory>
#include <thread>
#include <mutex>
#include <functional>
#include <vector>
#include <queue>
#include <condition_variable>
#include <stop_token>

// Global mutex for protecting output from simultaneous access by multiple threads
std::mutex outputMutex;

/*
Message has a sender id and a method apply
Apply is the function that applies the message to the actor by id
*/
class Message {
    public:
        // Constructor
        Message(int sender_id) : sender(sender_id) {}
        virtual ~Message() = default;

        // Public Apply
        virtual int Apply() {
            return _apply();
        }

    private:
        int sender; // The id for the actor that sent the message
        
        // Private _apply just returns success now
        // Eventually this will be more complicated, so it's not a pure virual function
        int _apply() {
            return EXIT_SUCCESS;
        }
};

/*
The actor object with an ID that can Recieve or Send messages
The constructor gives each actor it's own thread and access to scope
The destructor handles shutdown
Recieves prints to screen what happens in apply 
Send message puts a message to the queue of the actor
The process loop waits for new messages, if we have a message we recieve it's refrence
*/
class Actor {
    public:                            // Start the actor's thread
        Actor(int id) : actor_ref(id), worker([this](std::stop_token st) { processLoop(st); }) { }
        
        // Destructor that shuts down the thread
        ~Actor() {
            worker.request_stop();
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                // Signal thread to stop
            }
            cv.notify_one(); // Wake up the thread to exit
        }
        
        // Recieve a message and apply it from the Message object api
        void Receive(Message& m) {
            try {
                std::lock_guard<std::mutex> lock(outputMutex);
                std::cout << "Actor " << actor_ref << " received message. Apply result: " << m.Apply() << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Exception in Actor " << actor_ref << ": " << e.what() << std::endl;
            }
        }
    
        // Called by Engine to send a message
        void SendMessage(std::unique_ptr<Message> m) {
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                messageQueue.push_back(std::move(m));
            }
            cv.notify_one(); // Notify the thread a new message is available
        }
    
    private:
        int actor_ref; // Actor id
        std::deque<std::unique_ptr<Message>> messageQueue; // Queue for message objects, messages are executed sequentially
        std::jthread worker; // Thread we are using
        std::mutex queueMutex; // Mutex to modify the queue
        std::condition_variable cv; // Condition variable for notifying
    
        void processLoop(std::stop_token st) {
            while (!st.stop_requested()) {
                std::unique_ptr<Message> msg;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    // Wait for messages or shutdown signal
                    cv.wait(lock, [this, st] { return !messageQueue.empty() || st.stop_requested(); });
    
                    if (st.stop_requested() && messageQueue.empty()) {
                        return; // Exit if shutting down and no messages left
                    }
                    
                    // If we have a message get it so we can recieve it
                    if (!messageQueue.empty()) {
                        msg = std::move(messageQueue.front());
                        messageQueue.pop_front();
                    }
                }
                if (msg) {
                    Receive(*msg); // Process the message outside the lock
                }
            }
        }
};

/*
The engine spawns actors and sends them messages by their id
actor_map holds actors by their id
actor_message_map holds messages in a deque for each actor id

Spawn calls _spawn which adds a new refrence to an actor id to the actor_map
Poison calls _poison which 
Send calls _send which pushes back a message to the actor_message_map by id
*/
class Engine {
    public:
        // Public Spawn to return a new actor id
        int Spawn() {
            return _spawn();
        }

        // Public Poison to delete an actor
        void Poison(const int id) {
            _poison(id);
        }
    
        // Public Send to send a message to an actor by id
        void Send(const int id, std::unique_ptr<Message> m) {
            _send(id, std::move(m));
        }        
    
    private:
        std::unordered_map<int, std::unique_ptr<Actor>> actor_map; // maps id's to unique pointers to actors
        int next_id = 0;

        // Add the id to a map with a refrence to a new actor refrence
        int _spawn() {
            int id = next_id++;
            actor_map[id] = std::make_unique<Actor>(id);
            return id;
        }
        
        void _poison(const int id) {
            auto it = actor_map.find(id);
            if (it != actor_map.end()) {
                actor_map.erase(id); // Destructor handles shutdown
            } else {
                std::cerr << "Actor " << id << " not found." << std::endl;
            }
        }

        // Send a message with the Actor object
        void _send(const int id, std::unique_ptr<Message> m) {
            auto it = actor_map.find(id);
            if (it != actor_map.end()) {
                it->second->SendMessage(std::move(m));
            } else {
                std::cerr << "Actor " << id << " not found." << std::endl;
            }
        }        
    };

int main() {

    // Create Actor engine
    Engine e;
    int actor_1 = e.Spawn();
    int actor_2 = e.Spawn();
    
    e.Send(actor_1, std::make_unique<Message>(42));
    e.Send(actor_2, std::make_unique<Message>(42));
    
    // Give actors time to process
    std::this_thread::sleep_for(std::chrono::seconds(1));

    e.Poison(actor_1);
    e.Poison(actor_2);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}
