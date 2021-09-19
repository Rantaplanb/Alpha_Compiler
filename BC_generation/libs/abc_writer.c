#include "abc_writer.h"

#include "tcode_generation.h"

void write_abc(FILE *abc_file, ArrayList *instructions,
               ArrayList *userfunctions, ArrayList *libfuncs_used,
               ArrayList *const_numbers, ArrayList *const_strings) {
    unsigned number, i;

    // Magic number
    unsigned long long magic_number = 253876718360;
    fwrite(&magic_number, sizeof(magic_number), 1, abc_file);

    // Const strings
    if (const_strings) {
        number = const_strings->current_size;
        fwrite(&number, sizeof(number), 1, abc_file);
        for (i = 0; i < number; ++i) {
            char *string = (char *)const_strings->element[i];
            fwrite(string, sizeof(char), strlen(string) + 1, abc_file);
        }
    } else {
        number = 0;
        fwrite(&number, sizeof(number), 1, abc_file);
    }

    // Const numbers
    if (const_numbers) {
        number = const_numbers->current_size;
        fwrite(&number, sizeof(number), 1, abc_file);
        for (i = 0; i < number; ++i) {
            double *num = (double *)const_numbers->element[i];
            fwrite(num, sizeof(double), 1, abc_file);
        }
    } else {
        number = 0;
        fwrite(&number, sizeof(number), 1, abc_file);
    }

    // User functions
    if (userfunctions) {
        number = userfunctions->current_size;
        fwrite(&number, sizeof(number), 1, abc_file);
        for (i = 0; i < number; ++i) {
            int *address =
                (int *)&((SymbolTableEntry *)userfunctions->element[i])->taddress;
            int *localsize =
                (int *)&((SymbolTableEntry *)userfunctions->element[i])->offset;
            char *id = ((SymbolTableEntry *)userfunctions->element[i])->name;
            fwrite(address, sizeof(int), 1, abc_file);
            fwrite(localsize, sizeof(int), 1, abc_file);
            fwrite(id, sizeof(char), strlen(id) + 1, abc_file);
        }
    } else {
        number = 0;
        fwrite(&number, sizeof(number), 1, abc_file);
    }

    // Library functions
    if (libfuncs_used) {
        number = libfuncs_used->current_size;
        fwrite(&number, sizeof(number), 1, abc_file);
        for (i = 0; i < number; ++i) {
            char *id = (char *)libfuncs_used->element[i];
            fwrite(id, sizeof(char), strlen(id) + 1, abc_file);
        }
    } else {
        number = 0;
        fwrite(&number, sizeof(number), 1, abc_file);
    }

    // Instructions (first array of src lines, then actual instructions)
    if (instructions) {
        number = instructions->current_size;
        fwrite(&number, sizeof(number), 1, abc_file);
        for (i = 0; i < number; ++i) {
            unsigned *line =
                &((Instruction *)instructions->element[i])->srcLine;
            fwrite(line, sizeof(unsigned), 1, abc_file);
        }
        for (i = 0; i < number; ++i) {
            char *opcode =
                (char *)&((Instruction *)instructions->element[i])->opcode;
            char *result_type =
                (char *)&((Instruction *)instructions->element[i])->result.type;
            unsigned *result_val =
                (unsigned *)&((Instruction *)instructions->element[i])
                    ->result.val;
            char *arg1_type =
                (char *)&((Instruction *)instructions->element[i])->arg1.type;
            unsigned *arg1_val =
                (unsigned *)&((Instruction *)instructions->element[i])
                    ->arg1.val;
            char *arg2_type =
                (char *)&((Instruction *)instructions->element[i])->arg2.type;
            unsigned *arg2_val =
                (unsigned *)&((Instruction *)instructions->element[i])
                    ->arg2.val;
            fwrite(opcode, sizeof(char), 1, abc_file);
            fwrite(result_type, sizeof(char), 1, abc_file);
            fwrite(result_val, sizeof(unsigned), 1, abc_file);
            fwrite(arg1_type, sizeof(char), 1, abc_file);
            fwrite(arg1_val, sizeof(unsigned), 1, abc_file);
            fwrite(arg2_type, sizeof(char), 1, abc_file);
            fwrite(arg2_val, sizeof(unsigned), 1, abc_file);
        }
    } else {
        number = 0;
        fwrite(&number, sizeof(number), 1, abc_file);
    }

    return;
}
