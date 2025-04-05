def convert_model_to_c_array(model_path, output_header):
    with open(model_path, "rb") as f:
        tflite_model = f.read()

    hex_array = ",".join(["0x{:02x}".format(b) for b in tflite_model])
    c_code = f"const unsigned char model_data[] = {{{hex_array}}};"

    with open(output_header, "w") as f:
        f.write(c_code)

    print(f"Model converted to C array and saved as {output_header}")

convert_model_to_c_array("6dof_model-2.tflite", "6dof_model.h")
