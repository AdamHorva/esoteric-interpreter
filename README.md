# BitInterpreter

A C++ interpreter for a custom esoteric, bit-level programming language. 
Inspired by Brainf*ck, 
It operates on an infinite tape of bits and supports a compact instruction set for flipping, moving, input/output, and looping.

---

## Features

- Bit-based tape memory using `std::vector<bool>` or optionally `std::deque<bool>`.
- Command execution through an extensible `ICommand` interface.
- Dynamically growing memory in both directions.
- Loop detection and safe execution via step count limiter.
- Converts input/output to and from binary.
- Easy debugging via command-specific logging.
- Minimal error handling

---

## Instruction Set

| Command | Description                         |
|---------|-------------------------------------|
|   `+`   | Flip the current bit (0 → 1, 1 → 0) |
|   `>`   | Move the pointer right              |
|   `<`   | Move the pointer left               |
|   `,`   | Read a bit from input stream        |
|   `;`   | Write the current bit to output     |
|   `[`   | Jump forward if current bit is 0    |
|   `]`   | Jump back if current bit is 1       |

---

### How to run

./bit_interpreter program.txt "input_string" on   # enables debug
./bit_interpreter program.txt "input_string" off  # disables debug

---

### Note about the code:
(Tried to come up with descriptions and question why I used what and what was my goal)

Using factory pattern:
 - Command is read from the program and mapped to a specific command
 - Could have used switch case however decided to use manual implementation of factory pattern
 - will create a "factory-produced" objects
 - When my interpreter runs the command at

char c = state.code[state.ip];
if (command_map.contains(c)) {
    (command_map[c])(state);
}

 - It is dynamically dispatching the correct object based on the character
 - It is a factory like behaviour (Not a class), asking the factory for the right command at runtime

Advantages:
 - When adding new command we just add another struct command but we do not have to change the interpreter logic (in our case, the switch case)
 - Interpreter does not care how the commands are implemented
 - Readablity
 - Commands are resuable and could be used for mocked testing
 - Polymorphism

- Why dequeu for tape?
Used deque for the MoveLeftCmd command
 - inserting at the beginning can be expensive in case of a vector
 - deque is more effecient 

- Why unordered_map for the commands and std::function?

std::function:
 - My other solution was shared pointer, which is extendable, however
 - Performance wise this is better and commads are stateless (reduced overhead)

unordered_map:
 - This gives average-case O(1) time complexity for lookup, insertion and deletion
 - This is crucial since we can have millions of steps
 - Hashing cost is minimal since its just 1 char
 - We dont need order -> unordered
 - This is the other reason why I did not use switch case but factory pattern

Why was sharde_ptr considered?
 - I could store instances of any class implementing ICommand
 - shared_ptr avoids manual delete
 - Potentionally could use unique_ptr if no cpies of the map are made

Why stack for bracket validation?
 - When validiting brackets stacks are common practice
 - It uses LIFO, which is perfect for this issue
 - For each opening bracket, push them into the stack
 - For each closing bracket, check if the stack is not empty and if the top matches the current closing bracket
 - If the stack is empty at the end if the traversal, the expression is balanced, otherwise it is not.

Why bitset for input/output conversion
 - It packs/unpacks to/from characters
 - Handles binary encoding/decoding effectively

Infinite Loop Detection?
 - crude way for infinite loop detection
 - Could not find better solution to this
 - Maybe a clock but at the end it is the same problem
 - As far as I know doing this reliably is impossible (Halting problem)
 - In our case maybe we could use some kind of map structer where it can be checked what was visited
 - Feels a bit too much for something like this
 - Wanted to include it anyway

### Compilation

Small notes:
 - included settings.json for VSCode CMake Tools (I used this mainly, not the manual way like the bash command)

```bash
g++ -std=c++17 -o bit_interpreter main.cpp


