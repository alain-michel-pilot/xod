#pragma XOD evaluate_on_pin disable
#pragma XOD evaluate_on_pin enable input_UPD
#pragma XOD error_raise enable

struct State {
    bool received = false;
};

// clang-format off
{{ GENERATED_CODE }}
// clang-format on

void evaluate(Context ctx) {
    auto inet = getValue<input_INET>(ctx);
    emitValue<output_INETU0027>(ctx, inet);

    auto t = getValue<input_T>(ctx);
    auto state = getState(ctx);

    if (isInputDirty<input_UPD>(ctx) && !inet->isReceiving()) {
        setTimeout(ctx, t * 1000);
        return;
    }

    if (isTimedOut(ctx)) {
        if (inet->isReceiving()) {
            state->received = true;
            setTimeout(ctx, 0);
            emitValue<output_Y>(ctx, 1);
        } else if (!inet->isReceiving() && state->received) {
            state->received = false;
            emitValue<output_END>(ctx, 1);
        } else {
            raiseError(ctx);
        }
    }
}
