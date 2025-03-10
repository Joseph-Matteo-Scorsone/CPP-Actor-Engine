# 🧠 Actor Model in Modern C++

This is a minimal and educational implementation of the **Actor Model** in modern C++.

The core components include:
- **Engine**: Spawns, manages, and terminates actors.
- **Actors**: Lightweight, message-driven entities running on their own threads.
- **Messaging**: Simple message-passing between actors (currently minimal behavior).

> ⚠️ Note: Message handling is implemented, but actors currently do not process messages beyond receipt.

---

## 🚀 Features

- ✅ Actor creation & management
- ✅ Thread-safe message passing
- ✅ Graceful termination via "poison pills"

---

## 🛠️ Build & Run

Make sure you have a C++23-compatible compiler (e.g., `g++ 13+`), then run:

```bash
make run
