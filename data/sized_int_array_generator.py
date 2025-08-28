import re
import sys
import numpy
from pathlib import Path

if len(sys.argv) < 2:
    print(f"Usage: {sys.argv[0]} input_file [input_file_2]...")
    print("Each input file should be a list of integers. For each input file, an output C file is written containing code for a C-style unsigned integer array initalizer that packs the integers as close together as possible with a constant bit width, straddling byte boundaries. If it would reduce the bit width or if the values are signed, the input values are offset so the minimum in the output is 0. The output will contain the bit width of the result and the offset required to shift the values back to their original range.")

for input_param in sys.argv[1:]:
    try:
        print(f"Processing {input_param}")
        with open(input_param, 'r') as input_file:
            input_array = re.findall(r"-?\d+", input_file.read())
            input_array = [int(e) for e in input_array]
            offset = 0
            if any([
                min(input_array) < 0,
                (max(input_array) - min(input_array)).bit_length() < max(input_array).bit_length()
            ]):
                offset = min(input_array)
                input_array = [e - offset for e in input_array] # Translate all values so the minimum is 0
            bit_width = max(input_array).bit_length()

            output_array = numpy.packbits([int(bit) for e in input_array for bit in numpy.binary_repr(e, width=bit_width)])
            output_array = [f"{num:#0{4}x}" for num in output_array] # Convert to array of string hex representations padded to 2 digits

            output_str = f"uint8_t output[{len(output_array)}] = {{ "
            for i in range(len(output_array)):
                if i % 16 == 0: output_str = output_str[:-1] + "\n\t"
                output_str += output_array[i] + ", "
            output_str = output_str[:-2] + "\n};\n"
            input_path = Path(input_param)
            with open(input_path.stem + "_output.c", "w") as output_file:
                output_file.write(f"// Bit width: {bit_width}, offset: {offset}\n")
                output_file.write(output_str)
    except (FileNotFoundError, PermissionError, IOError, ValueError) as e:
        print(e)

