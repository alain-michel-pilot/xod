#pragma XOD error_raise enable

const char OK[] = "OK";
const char FAIL[] = "FAIL";
const char ERROR[] = "ERROR";
const char IPD[] = "IPD,";
const char CLOSED[] = "CLOSED";
const char CIFSR[] = "AT+CIFSR";
const char STAIP[] = "STAIP,\"";
const char CIPCLOSE[] = "AT+CIPCLOSE";


class TetherinInternet {
private:
#ifdef XOD_WASM_SERIAL_H
    WasmSerial_* _serial = &XOD_DEBUG_SERIAL;
#else
    HardwareSerial* _serial = &XOD_DEBUG_SERIAL;
#endif
    uint16_t _nodeId;
    uint16_t _pkgSize = 0;
    void printPrefix() {
        _serial->print("+XOD_INET:");
        _serial->print(transactionTime());
        _serial->print(":");
        _serial->print(_nodeId);
        _serial->print(":");
    }
    uint8_t readCmd(const char* text1, const char* text2 = nullptr, uint32_t timeout = 10000) {
        // setup buffers on stack & copy data from PROGMEM pointers
        char buf1[16] = { '\0' };
        char buf2[16] = { '\0' };
        if (text1 != nullptr)
            strcpy(buf1, text1);
        if (text2 != nullptr)
            strcpy(buf2, text2);
        uint8_t len1 = strlen(buf1);
        uint8_t len2 = strlen(buf2);
        uint8_t pos1 = 0;
        uint8_t pos2 = 0;
        // read chars until first match or timeout
        uint32_t stop = millis() + timeout;
        do {
            while (_serial->available()) {
                char c = _serial->read();
                pos1 = (c == buf1[pos1]) ? pos1 + 1 : 0;
                pos2 = (c == buf2[pos2]) ? pos2 + 1 : 0;
                if (len1 > 0 && pos1 == len1) {
                    return 1;
                }
                if (len2 > 0 && pos2 == len2) {
                    return 2;
                }
            }
        } while (millis() < stop);
        return 0;
    }
public:
    TetherinInternet(uint16_t nodeId) {
        _nodeId = nodeId;
    }
    bool kick() {
        printPrefix();
        _serial->println("AT");
        _serial->flush();
        uint8_t res = readCmd(OK, ERROR);
        return res == 1;
    }
    bool openTcp(XString host, uint32_t port, uint16_t keepAlive = 0) {
        printPrefix();
        // Command + connection type
        _serial->print("AT+CIPSTART=\"TCP\",\"");
        // Host
        for (auto it = host.iterate(); it; ++it)
            _serial->print((char)*it);
        // Port
        _serial->print("\",");
        _serial->print(port);
        // Delimiter
        _serial->print(",");
        // Keep alive
        _serial->println(keepAlive);
        _serial->flush();

        return readCmd(OK, ERROR) == 1;
    }
    bool send(XString req) {
        printPrefix();

        size_t reqLen = length(req);
        _serial->print("AT+CIPSEND=");
        _serial->println(reqLen);
        _serial->flush();

        bool prompt = readCmd(">", "link is not") == 1;
        if (!prompt) return false;

        bool nextLine = true;
        bool cr = false;
        for (auto it = req.iterate(); it; ++it) {
            if (nextLine) {
                printPrefix();
                nextLine = false;
            }
            if (*it == '\r') {
              cr = true;
              _serial->print("\\r");
            } else if (*it == '\n') {
                nextLine = true;
                _serial->print("\\n");
                if (cr) _serial->print('\r');
                _serial->print('\n');
            } else {
                _serial->print((char)*it);
            }
        }

        _serial->flush();

        return readCmd("SEND OK", nullptr, 5000) == 1;
    }
    bool isAvailable() {
        return _serial->available();
    }
    bool isReceiving() {
        return _pkgSize > 0;
    }
    bool receiveByte(char* outBuff, uint32_t timeout = 1000) {
        uint32_t stop = millis() + timeout;
        do {
            if (isAvailable() && isReceiving()) {
                *outBuff = _serial->read();
                _pkgSize--;
                return true;
            }
        } while (millis() < stop);
        return false;
    }
    void beginReceiving(uint16_t pkgSize) {
        _pkgSize = pkgSize - 1;
    }
    uint32_t getIP() {
        printPrefix();
        _serial->println(CIFSR);
        uint8_t code = readCmd(STAIP, ERROR);
        if (code == 1) {
            // found ip
            uint32_t ip[4];
            ip[0] = _serial->parseInt();
            _serial->read(); // .
            ip[1] = _serial->parseInt();
            _serial->read(); // .
            ip[2] = _serial->parseInt();
            _serial->read(); // .
            ip[3] = _serial->parseInt();

            if ((ip[0] | ip[1] | ip[2] | ip[3]) >= 0x100)
                return 0;

            return ip[0] + ip[1] * 0x100 + ip[2] * 0x10000ul + ip[3] * 0x1000000ul;
        }
        return 0;
    }
    bool close(uint8_t linkId) {
        // TODO: MUX
        printPrefix();
        _serial->println(CIPCLOSE);
        return readCmd(OK, ERROR) == 1;
    }
};

struct State {
    uint8_t mem[sizeof(TetherinInternet)];
    TetherinInternet* inet;
};

using Type = TetherinInternet*;

{{ GENERATED_CODE }}

void evaluate(Context ctx) {
    auto state = getState(ctx);
    state->inet = new (state->mem) TetherinInternet(getNodeId(ctx));
    emitValue<output_INET>(ctx, state->inet);
}
