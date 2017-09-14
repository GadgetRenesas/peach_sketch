/* ESP8266 Example
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ESP8266.h"

ESP8266 * ESP8266::instESP8266 = NULL;

ESP8266 * ESP8266::getESP8266Inst(PinName tx, PinName rx, bool debug)
{
    if (instESP8266 == NULL) {
        instESP8266 = new ESP8266(tx, rx, debug);
    } else {
        if (debug) {
            instESP8266->debugOn(debug);
        }
    }
    return instESP8266;
}

ESP8266::ESP8266(PinName tx, PinName rx, bool debug)
    : _serial(tx, rx, 1024), _parser(_serial), init_end(false)
    , _packets(0), _packets_end(&_packets)
    , _id_bits(0), _id_bits_close(0),_server_act(false)
{
    recv_waiting = false;
    _wifi_mode = 1;
    memset(_ids, 0, sizeof(_ids));
    memset(_cbs, 0, sizeof(_cbs));

    _serial.baud(115200);
    _parser.debugOn(debug);
    _serial.attach(Callback<void()>(this, &ESP8266::event));
}

void ESP8266::debugOn(bool debug)
{
    _parser.debugOn(debug);
}

bool ESP8266::startup()
{
    if (init_end) {
        return true;
    }

    bool success = reset()
        && _parser.send("AT+CWMODE=%d", _wifi_mode)
        && _parser.recv("OK")
        && _parser.send("AT+CIPMUX=1")
        && _parser.recv("OK");

    if (success) {
        _parser.oob("+IPD", this, &ESP8266::_packet_handler);
        _parser.oob("0,CONNECT", this, &ESP8266::_connect_handler_0);
        _parser.oob("1,CONNECT", this, &ESP8266::_connect_handler_1);
        _parser.oob("2,CONNECT", this, &ESP8266::_connect_handler_2);
        _parser.oob("3,CONNECT", this, &ESP8266::_connect_handler_3);
        _parser.oob("4,CONNECT", this, &ESP8266::_connect_handler_4);
        _parser.oob("0,CLOSED", this, &ESP8266::_closed_handler_0);
        _parser.oob("1,CLOSED", this, &ESP8266::_closed_handler_1);
        _parser.oob("2,CLOSED", this, &ESP8266::_closed_handler_2);
        _parser.oob("3,CLOSED", this, &ESP8266::_closed_handler_3);
        _parser.oob("4,CLOSED", this, &ESP8266::_closed_handler_4);
        init_end = true;
    }

    return success;
}

bool ESP8266::restart()
{
    bool success;

    _lock.lock();
    if (!init_end) {
        success = startup();
    } else {
        reset();
        _parser.setTimeout(5000);
        success = _parser.send("AT+CWMODE=%d", _wifi_mode)
               && _parser.recv("OK")
               && _parser.send("AT+CIPMUX=1")
               && _parser.recv("OK");
    }
    _lock.unlock();

    return success;
}

bool ESP8266::set_mode(int mode)
{
    //only 3 valid modes
    if (mode < 1 || mode > 3) {
        return false;
    }
    _wifi_mode = mode;
    return restart();
}

bool ESP8266::cre_server(int port)
{
    if (_server_act) {
        return false;
    }
    _lock.lock();
    startup();
    _parser.setTimeout(3000);
    if (!(_parser.send("AT+CIPSERVER=1,%d", port)
        && _parser.recv("OK"))) {
        _lock.unlock();
        return false;
    }
    _server_act = true;
    _lock.unlock();
    return true;
}

bool ESP8266::del_server()
{
    _lock.lock();
    startup();
    _parser.setTimeout(3000);
    if (!(_parser.send("AT+CIPSERVER=0")
        && _parser.recv("OK"))) {
        _lock.unlock();
        return false;
    }
    _server_act = false;
    _lock.unlock();
    return true;
}

void ESP8266::socket_handler(bool connect, int id)
{
    _lock.lock();
    startup();
    if (connect) {
        _id_bits |= (1 << id);
        if (_server_act) {
            _accept_id.push_back(id);
        }
    } else {
        _id_bits &= ~(1 << id);
        _id_bits_close |= (1 << id);
        if (_server_act) {
            for (size_t i = 0; i < _accept_id.size(); i++) {
                if (id == _accept_id[i]) {
                    _accept_id.erase(_accept_id.begin() + i);
                }
            }
        }
    }
    _lock.unlock();
}

void ESP8266::_connect_handler_0() { socket_handler(true, 0);  }
void ESP8266::_connect_handler_1() { socket_handler(true, 1);  }
void ESP8266::_connect_handler_2() { socket_handler(true, 2);  }
void ESP8266::_connect_handler_3() { socket_handler(true, 3);  }
void ESP8266::_connect_handler_4() { socket_handler(true, 4);  }
void ESP8266::_closed_handler_0()  { socket_handler(false, 0); }
void ESP8266::_closed_handler_1()  { socket_handler(false, 1); }
void ESP8266::_closed_handler_2()  { socket_handler(false, 2); }
void ESP8266::_closed_handler_3()  { socket_handler(false, 3); }
void ESP8266::_closed_handler_4()  { socket_handler(false, 4); }

bool ESP8266::accept(int * p_id)
{
    bool ret = false;

    while (!ret) {
        if (!_server_act) {
            break;
        }
        _lock.lock();
        startup();
        if (!_accept_id.empty()) {
            ret = true;
        } else {
            _parser.setTimeout(2);
            _parser.recv("%*,CONNECT");
            if (!_accept_id.empty()) {
                ret = true;
            }
        }
        if (ret) {
            *p_id = _accept_id[0];
            _accept_id.erase(_accept_id.begin());
        }
        _lock.unlock();
        Thread::wait(1);
    }

    if (ret) {
        for (int i = 0; i < 50; i++) {
            if ((_id_bits_close & (1 << *p_id)) == 0) {
                break;
            }
            Thread::wait(10);
        }
    }

    return ret;
}

bool ESP8266::reset(void)
{
    for (int i = 0; i < 2; i++) {
        if (_parser.send("AT+RST")
            && _parser.recv("OK\r\nready")) {
            return true;
        }
    }

    return false;
}

bool ESP8266::dhcp(bool enabled, int mode)
{
    bool ret;

    //only 3 valid modes
    if (mode < 0 || mode > 2) {
        return false;
    }

    _lock.lock();
    startup();
    _parser.setTimeout(3000);
    ret = _parser.send("AT+CWDHCP=%d,%d", enabled?1:0, mode)
       && _parser.recv("OK");
    _lock.unlock();
    return ret;
}

bool ESP8266::connect(const char *ap, const char *passPhrase)
{
    bool ret;

    _lock.lock();
    startup();
    _parser.setTimeout(3000);

    ret = _parser.send("AT+CWJAP=\"%s\",\"%s\"", ap, passPhrase)
        && _parser.recv("OK");
    _lock.unlock();
    return ret;
}

bool ESP8266::config_soft_ap(const char *ap, const char *passPhrase, uint8_t chl, uint8_t ecn)
{
    bool ret;

    _lock.lock();
    startup();
    _parser.setTimeout(3000);
    ret = _parser.send("AT+CWSAP=\"%s\",\"%s\",%hhu,%hhu", ap, passPhrase, chl, ecn)
       && _parser.recv("OK");
    _lock.unlock();
    return ret;
}

bool ESP8266::get_ssid(const char *ap)
{
    bool ret;

    _lock.lock();
    startup();
    _parser.setTimeout(500);
    ret = _parser.send("AT+CWJAP?")
       && _parser.recv("+CWJAP:\"%33[^\"]\",", ap)
       && _parser.recv("OK");
    _lock.unlock();
    return ret;
}

bool ESP8266::disconnect(void)
{
    return _parser.send("AT+CWQAP") && _parser.recv("OK");
}

const char *ESP8266::getIPAddress(void)
{
    if (!(_parser.send("AT+CIFSR")
        && _parser.recv("+CIFSR:STAIP,\"%15[^\"]\"", _ip_buffer)
        && _parser.recv("OK"))) {
        return 0;
    }

    return _ip_buffer;
}

const char *ESP8266::getIPAddress_ap(void)
{
    bool ret;

    _lock.lock();
    startup();
    _parser.setTimeout(3000);
    ret = _parser.send("AT+CIFSR")
       && _parser.recv("+CIFSR:APIP,\"%15[^\"]\"", _ip_buffer_ap)
       && _parser.recv("OK");
    _lock.unlock();
    if (!ret) {
        return 0;
    }
    return _ip_buffer_ap;
}

const char *ESP8266::getMACAddress(void)
{
    if (!(_parser.send("AT+CIFSR")
        && _parser.recv("+CIFSR:STAMAC,\"%17[^\"]\"", _mac_buffer)
        && _parser.recv("OK"))) {
        return 0;
    }

    return _mac_buffer;
}

const char *ESP8266::getMACAddress_ap(void)
{
    bool ret;

    _lock.lock();
    startup();
    _parser.setTimeout(3000);
    ret = _parser.send("AT+CIFSR")
       && _parser.recv("+CIFSR:APMAC,\"%17[^\"]\"", _mac_buffer_ap)
       && _parser.recv("OK");
    _lock.unlock();

    if (!ret) {
        return 0;
    }
    return _mac_buffer_ap;
}

const char *ESP8266::getGateway()
{
    if (!(_parser.send("AT+CIPSTA?")
        && _parser.recv("+CIPSTA:gateway:\"%15[^\"]\"", _gateway_buffer)
        && _parser.recv("OK"))) {
        return 0;
    }

    return _gateway_buffer;
}

const char *ESP8266::getGateway_ap()
{
    bool ret;

    _lock.lock();
    startup();
    _parser.setTimeout(3000);
    ret = _parser.send("AT+CIPAP?")
       && _parser.recv("+CIPAP:gateway:\"%15[^\"]\"", _gateway_buffer_ap)
       && _parser.recv("OK");
    _lock.unlock();

    if (!ret) {
        return 0;
    }
    return _gateway_buffer_ap;
}

const char *ESP8266::getNetmask()
{
    if (!(_parser.send("AT+CIPSTA?")
        && _parser.recv("+CIPSTA:netmask:\"%15[^\"]\"", _netmask_buffer)
        && _parser.recv("OK"))) {
        return 0;
    }

    return _netmask_buffer;
}

const char *ESP8266::getNetmask_ap()
{
    bool ret;

    _lock.lock();
    startup();
    _parser.setTimeout(3000);
    ret = _parser.send("AT+CIPAP?")
       && _parser.recv("+CIPAP:netmask:\"%15[^\"]\"", _netmask_buffer_ap)
       && _parser.recv("OK");
    _lock.unlock();

    if (!ret) {
        return 0;
    }
    return _netmask_buffer_ap;
}

int8_t ESP8266::getRSSI()
{
    int8_t rssi;
    char bssid[18];

   if (!(_parser.send("AT+CWJAP?")
        && _parser.recv("+CWJAP:\"%*[^\"]\",\"%17[^\"]\"", bssid)
        && _parser.recv("OK"))) {
        return 0;
    }

    if (!(_parser.send("AT+CWLAP=\"\",\"%s\",", bssid)
        && _parser.recv("+CWLAP:(%*d,\"%*[^\"]\",%hhd,", &rssi)
        && _parser.recv("OK"))) {
        return 0;
    }

    return rssi;
}

bool ESP8266::isConnected(void)
{
    return getIPAddress() != 0;
}

int ESP8266::scan(WiFiAccessPoint *res, unsigned limit)
{
    unsigned cnt = 0;
    nsapi_wifi_ap_t ap;

    if (!_parser.send("AT+CWLAP")) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    while (recv_ap(&ap)) {
        if (cnt < limit) {
            res[cnt] = WiFiAccessPoint(ap);
        }

        cnt++;
        if (limit != 0 && cnt >= limit) {
            break;
        }
    }

    return cnt;
}

bool ESP8266::open(const char *type, int id, const char* addr, int port)
{
    bool ret;

    //IDs only 0-4
    if (id > 4) {
        return false;
    }

    _lock.lock();
    startup();
    _parser.setTimeout(500);
    ret = _parser.send("AT+CIPSTART=%d,\"%s\",\"%s\",%d", id, type, addr, port)
       && _parser.recv("OK");
    _lock.unlock();

    return ret;
}

bool ESP8266::send(int id, const void *data, uint32_t amount)
{
    uint32_t send_size;
    bool ret;
    int error_cnt = 0;
    int index = 0;

    if (amount == 0) {
        return true;
    }

    //May take a second try if device is busy
    while (error_cnt < 2) {
        if (((_id_bits & (1 << id)) == 0)
         || ((_id_bits_close & (1 << id)) != 0)) {
            return false;
        }
        send_size = amount;
        if (send_size > 2048) {
            send_size = 2048;
        }
        _lock.lock();
        startup();
        _parser.setTimeout(1500);
        ret = _parser.send("AT+CIPSEND=%d,%d", id, send_size)
           && _parser.recv(">")
           && (_parser.write((char*)data + index, (int)send_size) >= 0)
           && _parser.recv("SEND OK");
        _lock.unlock();
        if (ret) {
            amount -= send_size;
            index += send_size;
            error_cnt = 0;
            if (amount == 0) {
                return true;
            }
        } else {
            error_cnt++;
        }
    }

    return false;
}

void ESP8266::_packet_handler()
{
    int id;
    uint32_t amount;
    int tmp_timeout;

    startup();
    // parse out the packet
    if (!_parser.recv(",%d,%d:", &id, &amount)) {
        return;
    }

    struct packet *packet = (struct packet*)malloc(
            sizeof(struct packet) + amount);
    if (!packet) {
        return;
    }

    packet->id = id;
    packet->len = amount;
    packet->next = 0;
    packet->index = 0;

    tmp_timeout = _parser.getTimeout();
    _parser.setTimeout(500);
    if (!(_parser.read((char*)(packet + 1), amount))) {
        free(packet);
        _parser.setTimeout(tmp_timeout);
        return;
    }
    _parser.setTimeout(tmp_timeout);

    // append to packet list
    *_packets_end = packet;
    _packets_end = &packet->next;
}

int32_t ESP8266::recv(int id, void *data, uint32_t amount)
{
    bool retry = false;

    while (true) {
        // check if any packets are ready for us
        for (struct packet **p = &_packets; *p; p = &(*p)->next) {
            if ((*p)->id == id) {
                struct packet *q = *p;

                if (q->len <= amount) { // Return and remove full packet
                    memcpy(data, (uint8_t*)(q+1) + q->index, q->len);

                    if (_packets_end == &(*p)->next) {
                        _packets_end = p;
                    }
                    *p = (*p)->next;

                    uint32_t len = q->len;
                    free(q);
                    return len;
                } else { // return only partial packet
                    memcpy(data, (uint8_t*)(q+1) + q->index, amount);

                    q->len -= amount;
                    q->index += amount;

                    return amount;
                }
            }
        }

        // Wait for inbound packet
        _lock.lock();
        startup();
        _parser.setTimeout(2);
        if (!_parser.recv("OK")) {
            _lock.unlock();
            if (retry) {
                if (((_id_bits & (1 << id)) == 0)
                 || ((_id_bits_close & (1 << id)) != 0)) {
                    return -2;
                } else {
                    recv_waiting = true;
                    return -1;
                }
            }
            retry = true;
        }
        _lock.unlock();
    }
}

bool ESP8266::close(int id, bool wait_close)
{
    if (wait_close) {
        for (int j = 0; j < 50; j++) {
            _lock.lock();
            startup();
            _parser.setTimeout(5);
            _parser.recv("%*,CLOSED");
            if (((_id_bits & (1 << id)) == 0)
             || ((_id_bits_close & (1 << id)) != 0)) {
                _lock.unlock();
                _id_bits_close &= ~(1 << id);
                _ids[id] = false;
                return true;
            }
            _lock.unlock();
            Thread::wait(10);
        }
    }

    //May take a second try if device is busy
    for (unsigned i = 0; i < 2; i++) {
        _lock.lock();
        startup();
        _parser.setTimeout(500);
        if (_parser.send("AT+CIPCLOSE=%d", id)
            && _parser.recv("OK")) {
            _lock.unlock();
            _id_bits_close &= ~(1 << id);
            _ids[id] = false;
            return true;
        }
        _lock.unlock();
    }

    _ids[id] = false;
    return false;
}

void ESP8266::setTimeout(uint32_t timeout_ms)
{
    _parser.setTimeout(timeout_ms);
}

bool ESP8266::readable()
{
    return _serial.readable();
}

bool ESP8266::writeable()
{
    return _serial.writeable();
}

void ESP8266::attach(int id, void (*callback)(void *), void *data)
{
    _cbs[id].callback = callback;
    _cbs[id].data = data;
}

int ESP8266::get_free_id()
{
    // Look for an unused socket
    int id = -1;

    for (int i = 0; i < ESP8266_SOCKET_COUNT; i++) {
        if ((!_ids[i]) && ((_id_bits & (1 << i)) == 0)) {
            id = i;
            _ids[i] = true;
            break;
        }
    }

    return id;
}

void ESP8266::event() {
    if (!recv_waiting) {
        return;
    }
    recv_waiting = false;

    for (int i = 0; i < ESP8266_SOCKET_COUNT; i++) {
        if (_cbs[i].callback) {
            _cbs[i].callback(_cbs[i].data);
        }
    }
}

bool ESP8266::recv_ap(nsapi_wifi_ap_t *ap)
{
    int sec;
    bool ret = _parser.recv("+CWLAP:(%d,\"%32[^\"]\",%hhd,\"%hhx:%hhx:%hhx:%hhx:%hhx:%hhx\",%d", &sec, ap->ssid,
                            &ap->rssi, &ap->bssid[0], &ap->bssid[1], &ap->bssid[2], &ap->bssid[3], &ap->bssid[4],
                            &ap->bssid[5], &ap->channel);

    ap->security = sec < 5 ? (nsapi_security_t)sec : NSAPI_SECURITY_UNKNOWN;

    return ret;
}
