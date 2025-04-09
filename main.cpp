#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <functional>
#include <bitset>
#include <stack>
#include <deque>
#include <fstream>

constexpr size_t INITIAL_TAPE_SIZE = 1024;
constexpr size_t MAX_EXECUTION_STEPS = 10000000; // Prevent infinite loops

bool DEBUG_ENABLED = true;

struct State
{
    std::deque<bool> tape;
    size_t ptr = INITIAL_TAPE_SIZE / 2;
    size_t idx_ptr = 0;
    size_t input_ptr = 0;
    size_t mem_used = INITIAL_TAPE_SIZE;
    size_t left_pad = INITIAL_TAPE_SIZE / 2;
    size_t step_count = 0;
    std::vector<bool> input_bits;
    std::vector<bool> output_bits;
    std::string code;
    std::unordered_map<size_t, size_t> loop_map;

    State(const std::string &code_, const std::vector<bool> &input_bits_)
        : tape(INITIAL_TAPE_SIZE, false), input_bits(input_bits_), code(std::move(code_)) {}
};

struct ICommand
{
    virtual void operator()(State &state) = 0;
    virtual ~ICommand() = default;
};

struct PlusCmd : ICommand
{
    void operator()(State &s) override
    {
        s.tape[s.ptr] = !s.tape[s.ptr];

        if (DEBUG_ENABLED)
            std::cout << "[+] Flip bit at " << s.ptr << " -> " << s.tape[s.ptr] << "\n";
    }
};

struct InputCmd : ICommand
{
    void operator()(State &s) override
    {
        s.tape[s.ptr] = (s.input_ptr < s.input_bits.size()) ? s.input_bits[s.input_ptr++] : 0;
        if (DEBUG_ENABLED)
            std::cout << "[,] Read input bit -> " << s.tape[s.ptr] << "\n";
    }
};

struct OutputCmd : ICommand
{
    void operator()(State &s) override
    {
        s.output_bits.push_back(s.tape[s.ptr]);
        if (DEBUG_ENABLED)
            std::cout << "[;] Output bit -> " << s.tape[s.ptr] << "\n";
    }
};

struct MoveRightCmd : ICommand
{
    void operator()(State &s) override
    {
        ++s.ptr;
        if (s.ptr >= s.tape.size())
        {
            s.tape.resize(s.tape.size() + INITIAL_TAPE_SIZE, false);
            s.mem_used = s.tape.size();
            if (DEBUG_ENABLED)
                std::cout << "[>] Extended tape to " << s.mem_used << " bits\n";
        }
        else
        {
            if (DEBUG_ENABLED)
                std::cout << "[>] Move right to " << s.ptr << "\n";
        }
    }
};

struct MoveLeftCmd : ICommand
{
    void operator()(State &s) override
    {
        if (s.ptr == 0)
        {
            s.tape.insert(s.tape.begin(), INITIAL_TAPE_SIZE, false);
            s.ptr += INITIAL_TAPE_SIZE;
            s.left_pad += INITIAL_TAPE_SIZE;
            s.mem_used = s.tape.size();
            if (DEBUG_ENABLED)
                std::cout << "[<] Extended tape to " << s.mem_used << " bits (left pad)\n";
        }
        --s.ptr;
        if (DEBUG_ENABLED)
            std::cout << "[<] Move left to " << s.ptr << "\n";
    }
};

struct JumpForwardCmd : ICommand
{
    void operator()(State &s) override
    {
        if (!s.tape[s.ptr])
        {
            if (DEBUG_ENABLED)
                std::cout << "[[ Jump forward from " << s.idx_ptr << " to " << s.loop_map[s.idx_ptr] << "\n";
            s.idx_ptr = s.loop_map[s.idx_ptr];
        }
    }
};

struct JumpBackwardCmd : ICommand
{
    void operator()(State &s) override
    {
        if (s.tape[s.ptr])
        {
            if (DEBUG_ENABLED)
                std::cout << "]] Jump backward from " << s.idx_ptr << " to " << s.loop_map[s.idx_ptr] << "\n";

            s.idx_ptr = s.loop_map[s.idx_ptr];
        }
    }
};

class BitInterpreter
{
public:
    std::vector<bool> interpret(const std::string &code, const std::vector<bool> &input_bits)
    {
        State state(code, input_bits);
        build_jump_map(state);
        setup_commands();

        while (state.idx_ptr < state.code.size())
        {
            if (++state.step_count > MAX_EXECUTION_STEPS)
            {
                throw std::runtime_error("Execution stopped: possible infinite loop or too many instructions.");
                break;
            }

            char c = state.code[state.idx_ptr];

            // Use find to check for the existence of the command in the map
            auto it = command_map.find(c);
            if (it != command_map.end())
            {
                (it->second)(state); // Call the corresponding command
            }

            state.idx_ptr++;
        }
        if (DEBUG_ENABLED)
            std::cout << "[MEM] Final memory used: " << state.mem_used << " bits\n";

        return state.output_bits;
    }

private:
    std::unordered_map<char, std::function<void(State&)>> command_map;

    void setup_commands()
    {
        command_map['+'] = PlusCmd{};
        command_map[','] = InputCmd{};
        command_map[';'] = OutputCmd{};
        command_map['>'] = MoveRightCmd{};
        command_map['<'] = MoveLeftCmd{};
        command_map['['] = JumpForwardCmd{};
        command_map[']'] = JumpBackwardCmd{};
    }

    /*
        Build the jump map and validate square brackets
    */
    void build_jump_map(State &s)
    {
        // Stack is commonly used for this problem
        std::stack<size_t> stack; // Stack to keep track of '[' positions
        for (size_t i = 0; i < s.code.size(); ++i)
        {
            if (s.code[i] == '[')
            {
                stack.push(i); // Push the position of '[' onto the stack
            }
            else if (s.code[i] == ']')
            {
                if (stack.empty())
                {
                    std::cerr << "Unmatched ']' at position " << i << "\n";
                    std::exit(1);
                }
                size_t start = stack.top(); // Get the position of the matching '['
                stack.pop();                // Pop the top of the stack
                s.loop_map[start] = i;      // Map the '[' to ']'
                s.loop_map[i] = start;      // Map the ']' to '['
            }
        }

        // If the stack is not empty, there are unmatched '['
        if (!stack.empty())
        {
            std::cerr << "Unmatched '[' at position " << stack.top() << "\n";
            std::exit(1);
        }
    }
};

std::vector<bool> to_bit_stream(const std::string &input)
{
    std::vector<bool> bits;
    for (char c : input)
    {
        std::bitset<8> b(static_cast<unsigned char>(c));
        for (int i = 0; i < 8; ++i)
            bits.push_back(b[i]); // Little-endian
    }
    return bits;
}

std::string from_bit_stream(const std::vector<bool> &bits)
{
    std::string output;
    for (size_t i = 0; i < bits.size(); i += 8)
    {
        std::bitset<8> b;
        for (size_t j = 0; j < 8 && (i + j) < bits.size(); ++j)
            b[j] = bits[i + j];
        output += static_cast<char>(b.to_ulong());
    }
    return output;
}

int main(int argc, char *argv[])
{
    try
    {
        if (argc < 3 || argc > 4)
        {
            std::cerr << "Usage: " << argv[0] << " <program_file.txt> <input_string> [debug: on/off]\n";
            return 1;
        }

        std::ifstream file(argv[1]);
        if (!file)
        {
            std::cerr << "Failed to open program file: " << argv[1] << "\n";
            return 1;
        }

        std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        std::string input_string = argv[2];

        if (argc == 4) {
            std::string debug_arg = argv[3];
            DEBUG_ENABLED = (debug_arg == "on" || debug_arg == "ON" || debug_arg == "1");
        }

        BitInterpreter interpreter;
        auto input_bits = to_bit_stream(input_string);
        auto output_bits = interpreter.interpret(code, input_bits);
        std::cout << "[OUTPUT]:\n" << from_bit_stream(output_bits) << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[EXCEPTION] " << e.what() << std::endl;
    }
    return 0;
}
