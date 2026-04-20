# Chesstract

Chesstract is a desktop chess application built with **Modern C++** and **SFML**.

This project started as a personal journey to dive deep into C++ and game state management. What began as a simple grid has evolved into a fully functional chess board with interactive move history and PGN generation!

![Chesstract Screenshot](screenshot.png)

## Architecture & SoC (Separation of Concerns)
The project follows a strict **Separation of Concerns** architecture:
* **ChessRules (The Brain):** A standalone, platform-independent chess engine that handles all legal moves, board state, and rules (Castling, En Passant, Promotion).
* **Board (The UI):** A specialized rendering layer that manages textures, sprites, animations, and user input (Drag & Drop, Shortcuts).

## Current Features (update: April 2026)
* **Full Chess Logic:** Complete implementation of FIDE rules, including sophisticated move disambiguation (e.g., `R3h2`, `Ngf3`).
* **PGN Generation & Export:** Real-time PGN logging in the console and native file export via Windows dialogs.
* **Navigation System:** Travel through time! Use arrow keys to navigate move history or `Undo` to revert mistakes.
* **Promotion Engine:** Interactive pawn promotion menu (Queen, Rook, Bishop, Knight).
* **Visual Feedback:** Dynamic move highlights, pulsating "Danger Zones" for Kings in check, and smooth drag-and-drop mechanics.
* **SQLite Database Integration:** Persistent game storage. Save your matches to a local collection for future analysis.
* **Clipboard Utility:** Instantly copy move sequences to your clipboard for quick analysis on external platforms like Lichess or Chess.com

## Shortcuts
| Key | Action |
| :--- | :--- |
| **Left / Right** | Navigate through move history |
| **Up / Down** | Jump to start / Jump to current move |
| **U** | Undo last move |
| **S** | Save PGN to file |
| **K** | Copy raw move sequence to Clipboard |
| **P** | Print PGN to console |
| **F** | Flip the board (White/Black view) |
| **C** | Toggle coordinates |

## Roadmap (Updated)
* [x] **Architecture Refactoring:** Successfully decoupled Logic and UI into `ChessRules` and `Board`.
* [ ] **AI Opponent:** Implementation of a basic bot using Minimax with Alpha-Beta pruning.
* [ ] **Network Play:** Basic socket programming to allow two players over a local network.
* [x] **Database Integration:** Local SQLite storage for game archiving and opening sequences.
* [x] **Clipboard Features:** Quick-copy functionality for move notation.

## Built With
* **C++20** (Modern C++ Standard)
* **SFML 2.6+** (Simple and Fast Multimedia Library)