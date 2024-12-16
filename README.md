# SQLite C Driver

Hey there! 👋 Welcome to **SQLite C Driver**, a simple yet powerful project, this was meant to be a college's project that interfaces with csv files .. but honestly why not have some fun with uni adssignments right?. It demonstrates how to interact with SQLite databases using C. Whether you're a database enthusiast or just curious about how low-level database operations work, this project is a great starting point.

## What's Inside?

Here's a quick look at what you'll find in this project:

### Key Files
- **`database.c` and `database.h`**: Contains the core logic for connecting to and interacting with an SQLite database.
- **`functions.c` and `functions.h`**: Includes utility functions to support database operations.
- **`main.c`**: The entry point of the application where everything comes together.
- **`zerbew.db`**: A sample SQLite database to test and play around with.
- **`host.txt`**: Contains host-related commands (How to run it on your local machine).

## Features

- **Database Connectivity**: Establish a connection to an SQLite database and perform CRUD operations.
- **Utility Functions**: Reusable functions for common database tasks.
- **Sample Database**: A preloaded SQLite file (`zerbew.db`) for testing and exploration.

## Design Patterns Used

Here are the design patterns that make this project tick:

- **Singleton Pattern**: Ensures there's only one database connection instance at any given time.
- **Factory Pattern**: Simplifies the creation of database-related objects and structures.
- **RAII (Resource Acquisition Is Initialization)**: Ensures proper cleanup of resources (like database connections) when they're no longer needed.

## Sticking to SOLID Principles

I've kept things simple but structured to follow SOLID principles:

1. **Single Responsibility Principle**: Each file and function serves a single purpose (e.g., `database.c` handles database logic, `functions.c` provides utilities).
2. **Open/Closed Principle**: The code is open for extension, so you can easily add new database features without modifying existing functionality.
3. **Liskov Substitution Principle**: Components can be swapped out without breaking the app (e.g., replacing the database file or functions).
4. **Interface Segregation Principle**: Functions are modular and focus on specific tasks, avoiding bloated headers.
5. **Dependency Inversion Principle**: High-level logic (like `main.c`) depends on abstractions (`database.h`), not concrete implementations.
