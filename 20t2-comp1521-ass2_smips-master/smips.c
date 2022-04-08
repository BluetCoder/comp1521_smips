// COMP1521 20T2 --- assignment 2: smips, Simple MIPS
//
// Written by z5259375, August 2020.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Number of commands allowed
#define MAX_SIZE 1000

// Integer used to identify a command
#define UNKNOWN_COMMAND 0
#define ADD 1
#define SUB 2
#define AND 3
#define OR 4
#define SLT 5
#define MUL 6
#define BEQ 7
#define BNE 8
#define ADDI 9
#define SLTI 10
#define ANDI 11
#define ORI 12
#define LUI 13
#define SYSCALL 14

uint32_t hex_to_bin(int hex_command[8], int size_of_command);
uint32_t hex_bit_converter(int hex_bit);
int identify_command(uint32_t bin_command);
void print_command(int command, uint32_t bin_command);
int identify_register(int shift, uint32_t bin_command);
int identify_immediate(uint32_t bin_command);
int do_command(int command, int registers[32], uint32_t bin_command);

int main(int argc, char *argv[]){
    // Checks if it gives a name for a file
    if (argc < 2) {
        fprintf(stderr, "Need filename with hex codes for mips instructions\n");
        return 1;
    }
    int registers[32] = {0}; 
    int hex_array[8] = {0};
    uint32_t bin_command_array[MAX_SIZE] = {0};
    int command_array[MAX_SIZE] = {0};
    FILE *file_to_read = fopen(argv[1], "r");
    
    // Checks if the file exists, else closes the program. 
    if (file_to_read == NULL) {
        fprintf(stderr, "No such file or directory: '%s'\n", argv[1]);
        return 1;
    }
    int c = fgetc(file_to_read);
    int num_of_commands = 0;
    
    // The loop goes through the hex commands. It stores the hex command into 
    // an array then converts it to a binary command. It stores the type of 
    // command in command_array[]. It also checks if the command is valid.
    for (int i = 0; c != EOF; i++) {
        if (i >= MAX_SIZE) {
            printf("File too long: '%s'\n", argv[1]);
            return 1;
        }
        int hex_size = 0; 
        while (hex_size < 8 && c != '\n' && c != EOF) {
            hex_array[hex_size] = c;
            c = fgetc(file_to_read);
            hex_size++;
        }
        if (c == '\n') {
            c = fgetc(file_to_read);
        }
        uint32_t bin_command = hex_to_bin(hex_array, hex_size);
        int command = identify_command(bin_command);
        if (command == UNKNOWN_COMMAND) {
            printf("%s:%d: invalid instruction code: %08x\n", argv[1], i+1, 
            command);
            return 1;
        }
        bin_command_array[num_of_commands] = bin_command;
        command_array[num_of_commands] = command;
        num_of_commands ++; 
    }
    
    // The loop prints the command by going through the binary array and 
    // command array. 
    printf("Program\n");
    for (int i = 0; i < num_of_commands; i++) {
        printf("%3d", i);
        print_command(command_array[i], bin_command_array[i]);
    }
    
    // The loop prints the output of the syscalls by going through the binary 
    // array and command array. It executes the MIPS commands and prints
    // the output produced by the syscalls.
    printf("Output\n");
    for (int i = 0; i < num_of_commands; i++) {
        int I_bne_beq = do_command(command_array[i], registers, 
        bin_command_array[i]);
        registers[0] = 0;
        if (command_array[i] == BEQ || command_array[i] == BNE) {
            if (I_bne_beq != 0) {
                i += I_bne_beq - 1;
            }
        } else if (command_array[i] == SYSCALL) {
            if (registers[2] == 1) {
                printf("%d", registers[4]); 
            } else if (registers[2] == 10) {
                break; 
            } else if (registers[2] == 11) {
                printf("%c", registers[4]); 
            } else {
                printf("Unknown system call: %d\n", registers[2]);
                break;    
            }
        }
    }
    
    // The loop prints register values when the program terminates. It prints
    // whats in the register if the register is not zero. Registers are stored 
    // in an array.
    printf("Registers After Execution\n");
    for (int i = 0; i < 32; i++) {
        if (registers[i] != 0) {
            printf("$%-2d = %d\n", i, registers[i]);
        }
    }
    fclose(file_to_read);
    return 0;
}
// Identifies what the command is by returning a number corresponding to the 
// command. If the command is unknown, it returns a 0. E.g Add bit pattern 
// is 000000ssssstttttddddd00000100000 first_identification is 000000 which is 
// the first 6 bits on the right. The second_identification is 100000 which is
// bits on the right that helps identify the command. 
int identify_command(uint32_t bin_command){
    int command = UNKNOWN_COMMAND; 
    uint32_t first_identification = bin_command >> 26;
    uint32_t second_identification;
    if (first_identification == 0) {
        second_identification = 0xFF;
        second_identification &= bin_command;
        if (second_identification == 0xC) {
            command = SYSCALL;
        } else if (second_identification == 0x20) {
            command = ADD;
        } else if (second_identification == 0x22) {
            command = SUB;
        } else if (second_identification == 0x24) {
            command = AND;
        } else if (second_identification == 0x25) {
            command = OR;
        } else if (second_identification == 0x2A) {
            command = SLT;
        }    
    } else if (first_identification == 0x1C) {
        second_identification = 0xF;
        second_identification &= bin_command;
        if (second_identification == 0x2) {
            command = MUL;
        }    
    } else if (first_identification == 0x4) {
        command = BEQ;    
    } else if (first_identification == 0x5) {
        command = BNE;    
    } else if (first_identification == 0x8) {
        command = ADDI;    
    } else if (first_identification == 0xA) {
        command = SLTI;    
    } else if (first_identification == 0xC) {
        command = ANDI;    
    } else if (first_identification == 0xD) {
        command = ORI;    
    } else if (first_identification == 0xF) {
        second_identification = bin_command >> 21;
        if (second_identification == 0x1E0) {
            command = LUI;
        }    
    }
    return command;
}
// Converts the hex command stored in an array and returns the command in binary
uint32_t hex_to_bin(int hex_command[8], int size_of_command) {
    uint32_t binary_command = 0;
    for (int i = size_of_command - 1; i >= 0; i--) {
        uint32_t four_bits = hex_bit_converter(hex_command[i]); 
        binary_command |= four_bits << ((size_of_command - 1)*4 -i*4);
    }
    return binary_command;
}

// Converts the hex character in the array to the corresponding 4 bits
uint32_t hex_bit_converter(int hex_bit){
    uint32_t hex_bit_bin = 0;
    if (hex_bit == '0') {
        return hex_bit_bin;
    } else if (hex_bit == '1') {
        hex_bit_bin = 1;
    } else if (hex_bit == '2') {
        hex_bit_bin = 2;
    } else if (hex_bit == '3') {
        hex_bit_bin = 3;
    } else if (hex_bit == '4') {
        hex_bit_bin = 4;
    } else if (hex_bit == '5') {
        hex_bit_bin = 5;
    } else if (hex_bit == '6') {
        hex_bit_bin = 6;
    } else if (hex_bit == '7') {
        hex_bit_bin = 7;
    } else if (hex_bit == '8') {
        hex_bit_bin = 8;
    } else if (hex_bit == '9') {
        hex_bit_bin = 9;
    } else if (hex_bit == 'a') {
        hex_bit_bin = 10;
    } else if (hex_bit == 'b') {
        hex_bit_bin = 11;
    } else if (hex_bit == 'c') {
        hex_bit_bin = 12;
    } else if (hex_bit == 'd') {
        hex_bit_bin = 13;
    } else if (hex_bit == 'e') {
        hex_bit_bin = 14;
    } else if (hex_bit == 'f') {
        hex_bit_bin = 15;
    }
    return hex_bit_bin;
}

// Prints the commands corresponding to each instruction code. 
void print_command(int command, uint32_t bin_command){       
    if (command == ADD) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int d = identify_register(11, bin_command);
        printf(": add  $%d, $%d, $%d\n", d, s, t);
    } else if (command == SUB) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int d = identify_register(11, bin_command);
        printf(": sub  $%d, $%d, $%d\n", d, s, t);
    } else if (command == AND) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int d = identify_register(11, bin_command);
        printf(": and  $%d, $%d, $%d\n", d, s, t);    
    } else if (command == OR) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int d = identify_register(11, bin_command);
        printf(": or   $%d, $%d, $%d\n", d, s, t);    
    } else if (command == SLT) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int d = identify_register(11, bin_command);
        printf(": slt  $%d, $%d, $%d\n", d, s, t);    
    } else if (command == MUL) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int d = identify_register(11, bin_command);
        printf(": mul  $%d, $%d, $%d\n", d, s, t);    
    } else if (command == BEQ) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int I = identify_immediate(bin_command);
        printf(": beq  $%d, $%d, %d\n", s, t, I);   
    } else if (command == BNE) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int I = identify_immediate(bin_command);
        printf(": bne  $%d, $%d, %d\n", s, t, I);      
    } else if (command == ADDI) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int I = identify_immediate(bin_command);
        printf(": addi $%d, $%d, %d\n", t, s, I);    
    } else if (command == SLTI) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int I = identify_immediate(bin_command);
        printf(": slti $%d, $%d, %d\n", t, s, I);    
    } else if (command == ANDI) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int I = identify_immediate(bin_command);
        printf(": andi $%d, $%d, %d\n", t, s, I);    
    } else if (command == ORI) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int I = identify_immediate(bin_command);
        printf(": ori  $%d, $%d, %d\n", t, s, I);    
    } else if (command == LUI) {
        int t = identify_register(16, bin_command);
        int I = identify_immediate(bin_command);
        printf(": lui  $%d, %d\n", t, I);    
    } else if (command == SYSCALL) {
        printf(": syscall\n");    
    }
}

// Excecutes the commands corresponding to each instruction code. Returns
// the immediate for bne and beq. Otherwise returns 0.
int do_command(int command, int registers[32], uint32_t bin_command){       
    if (command == ADD) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int d = identify_register(11, bin_command);
        registers[d] = registers[s] + registers[t];
    } else if (command == SUB) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int d = identify_register(11, bin_command);
        registers[d] = registers[s] - registers[t];
    } else if (command == AND) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int d = identify_register(11, bin_command);
        registers[d] = registers[s] & registers[t];   
    } else if (command == OR) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int d = identify_register(11, bin_command);
        registers[d] = registers[s] | registers[t];   
    } else if (command == SLT) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int d = identify_register(11, bin_command);
        if (registers[s] < registers[t]) {
            registers[d] = 1;
        } else {
            registers[d] = 0;
        }    
    } else if (command == MUL) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int d = identify_register(11, bin_command);
        registers[d] = registers[s] * registers[t];   
    }  else if (command == BEQ) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int I = identify_immediate(bin_command);  
        if (registers[s] == registers[t]) {
            return I;
        } else {
            return 0;
        }
    } else if (command == BNE) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int I = identify_immediate(bin_command);
        if (registers[s] != registers[t]) {
            return I;
        } else {
            return 0;
        }      
    } else if (command == ADDI) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int I = identify_immediate(bin_command);
        registers[t] = registers[s] + I;  
    } else if (command == SLTI) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int I = identify_immediate(bin_command);
        registers[t] = (registers[s] < I);   
    } else if (command == ANDI) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int I = identify_immediate(bin_command);
        registers[t] = registers[s] & I;    
    } else if (command == ORI) {
        int t = identify_register(16, bin_command);
        int s = identify_register(21, bin_command);
        int I = identify_immediate(bin_command);
        registers[t] = registers[s] | I;   
    } else if (command == LUI) {
        int t = identify_register(16, bin_command);
        int I = identify_immediate(bin_command);
        registers[t] = I << 16;   
    }
    return 0;
}

// Identifies the register used in the command. Returns the five bit 
// register as an int. 
int identify_register(int shift, uint32_t bin_command){
    uint32_t adr_register = bin_command >> shift; 
    uint32_t bitmask = 0xFF >> 3;
    adr_register &= bitmask;
    return (int) adr_register;
}

// Identifies the 16 bit signed immediate in the command. Returns the 16
// bit immediate as an int. 
int identify_immediate(uint32_t bin_command){
    int16_t immediate = bin_command & 0xFFFF; 
    return (int) immediate;
}
