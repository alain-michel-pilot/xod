#pragma XOD evaluate_on_pin disable
#pragma XOD evaluate_on_pin enable input_READ
#pragma XOD error_raise enable

struct State {
};

// clang-format off
{{ GENERATED_CODE }}
// clang-format on

void evaluate(Context ctx) {
    if (!isInputDirty<input_READ>(ctx))
        return;

    auto inet = getValue<input_INET>(ctx);

    char t[10000] = { 0 };
    uint16_t pos = 0;
    uint32_t stop = millis() + 5000;
    do {
        while(XOD_DEBUG_SERIAL.available()) {
            t[pos] = XOD_DEBUG_SERIAL.read();
            pos++;
        }
    } while (millis() < stop && !XOD_DEBUG_SERIAL.available());


    XOD_DEBUG_SERIAL.print("GOT: <");
    XOD_DEBUG_SERIAL.print(t);
    XOD_DEBUG_SERIAL.println(">");
    XOD_DEBUG_SERIAL.flush();
//     char r = '\0';
//     if (inet->receive(&r)) {
//         emitValue<output_CHAR>(ctx, (uint8_t)r);
//         emitValue<output_DONE>(ctx, 1);
//     } else {
//         raiseError(ctx);
//     }
}
