# Alert Integrations Research

This document explores the different ways to extend the M5 Cardputer Deauth Detector to send alerts to external systems beyond the built-in REST API webhook. Approaches are divided into two tiers:

- **Direct (on-device)** — The ESP32 itself sends the alert with an additional firmware library
- **Via relay** — The existing webhook endpoint forwards events to the target service using a small middleware script or third-party service

---

## Table of Contents

1. [SMTP / Email](#1-smtp--email)
2. [WhatsApp](#2-whatsapp)
3. [Telegram](#3-telegram)
4. [SMS via Twilio](#4-sms-via-twilio)
5. [ntfy.sh Push Notifications](#5-ntfysh-push-notifications)
6. [Pushover](#6-pushover)
7. [MQTT Broker](#7-mqtt-broker)
8. [PagerDuty](#8-pagerduty)
9. [AWS SNS / SES](#9-aws-sns--ses)
10. [Gotify (Self-hosted Push)](#10-gotify-self-hosted-push)
11. [Comparison Summary](#comparison-summary)

---

## 1. SMTP / Email

### Overview

Email via SMTP is one of the most universally supported notification channels. The ESP32 can send email directly using the [ESP-Mail-Client](https://github.com/mobizt/ESP-Mail-Client) library, or you can relay through any SMTP provider.

### Option A — Direct from device (ESP_Mail_Client library)

Add the library to `platformio.ini`:

```ini
lib_deps =
    mobizt/ESP Mail Client@^3.4.19
```

Firmware sketch (add to a new `SMTPReporter` class):

```cpp
#include <ESP_Mail_Client.h>

SMTPSession smtp;

void sendEmailAlert(const String& targetSSID, const String& attackerMAC,
                    int channel, const String& timestamp) {
    ESP_Mail_Session session;
    session.server.host_name = "smtp.gmail.com"; // or your SMTP server
    session.server.port = 587;
    session.login.email = "sender@gmail.com";
    session.login.password = "app-specific-password";
    session.login.user_domain = "";

    SMTP_Message message;
    message.sender.name = "Deauth Detector";
    message.sender.email = "sender@gmail.com";
    message.subject = "⚠️ Deauth Attack Detected";
    message.addRecipient("Owner", "owner@example.com");

    String body = "Attack detected at " + timestamp + "\n";
    body += "Target network:  " + targetSSID + "\n";
    body += "Attacker MAC:    " + attackerMAC + "\n";
    body += "Channel:         " + String(channel);
    message.text.content = body;

    smtp.connect(&session);
    MailClient.sendMail(&smtp, &message);
    smtp.closeSession();
}
```

> **Note:** Gmail requires an App Password if 2-Step Verification is enabled.  
> Office 365 / Outlook and generic SMTP servers work with the same API by changing `host_name` and `port`.

### Option B — Via relay middleware

Point the device API endpoint at a small relay server that calls your SMTP provider SDK:

```python
# relay.py — Python/Flask example using smtplib
import smtplib, ssl
from email.mime.text import MIMEText
from flask import Flask, request

app = Flask(__name__)

SMTP_HOST   = "smtp.gmail.com"
SMTP_PORT   = 587
SMTP_USER   = "sender@gmail.com"
SMTP_PASS   = "app-specific-password"
RECIPIENT   = "owner@example.com"

@app.route('/v1/alerts', methods=['POST'])
def relay():
    events = request.get_json()
    for e in events:
        body = (f"Attack detected at {e['timestamp']}\n"
                f"Target   : {e['target_ssid']} ({e['target_bssid']})\n"
                f"Attacker : {e['attacker_mac']}\n"
                f"Channel  : {e['channel']}  RSSI: {e['rssi']} dBm\n"
                f"Packets  : {e['packet_count']}")
        msg = MIMEText(body)
        msg['Subject'] = f"⚠️ Deauth Attack on {e['target_ssid']}"
        msg['From']    = SMTP_USER
        msg['To']      = RECIPIENT
        ctx = ssl.create_default_context()
        with smtplib.SMTP(SMTP_HOST, SMTP_PORT) as s:
            s.starttls(context=ctx)
            s.login(SMTP_USER, SMTP_PASS)
            s.send_message(msg)
    return {"received": len(events)}, 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
```

### SMTP Provider Options

| Provider | Free Tier | Notes |
|----------|-----------|-------|
| Gmail | 500 emails/day | Requires App Password |
| SendGrid | 100 emails/day | REST API also available |
| Mailgun | 1,000 emails/month | Simple REST API |
| Amazon SES | 62,000/month (from EC2) | Very cheap beyond free tier |
| Postmark | 100/month | Excellent deliverability |

### Pros / Cons

| ✅ Pros | ❌ Cons |
|--------|--------|
| Universal — every smartphone can receive email | Latency can be 10–60 seconds |
| Rich formatting (HTML email possible) | SMTP credentials on device is a security risk |
| Works without installing any app | May land in spam without proper DNS (SPF/DKIM) |

---

## 2. WhatsApp

WhatsApp does not offer an official free outbound messaging API, but several gateway options are available.

### Option A — CallMeBot (Free, no account needed)

[CallMeBot](https://www.callmebot.com/blog/free-api-whatsapp-messages/) provides a free WhatsApp gateway. A one-time activation is required (you send a WhatsApp message to their bot to get an API key).

**Config in `config.txt`:**

```json
"api": {
  "endpoint_url": "http://your-relay-server/whatsapp",
  "custom_header_name": "",
  "custom_header_value": ""
}
```

**Relay script:**

```python
import requests, urllib.parse
from flask import Flask, request

app = Flask(__name__)

CALLMEBOT_PHONE  = "15551234567"   # with country code, no +
CALLMEBOT_APIKEY = "123456"

@app.route('/whatsapp', methods=['POST'])
def relay():
    events = request.get_json()
    for e in events:
        text = (f"⚠️ *Deauth Attack Detected*\n"
                f"Network: {e['target_ssid']}\n"
                f"Attacker: `{e['attacker_mac']}`\n"
                f"Channel: {e['channel']} | RSSI: {e['rssi']} dBm\n"
                f"Time: {e['timestamp']}")
        url = (f"https://api.callmebot.com/whatsapp.php"
               f"?phone={CALLMEBOT_PHONE}"
               f"&text={urllib.parse.quote(text)}"
               f"&apikey={CALLMEBOT_APIKEY}")
        requests.get(url)
    return {"received": len(events)}, 200
```

### Option B — Twilio WhatsApp Business API

[Twilio](https://www.twilio.com/whatsapp) provides a reliable, production-grade WhatsApp API with a free sandbox.

```python
from twilio.rest import Client
from flask import Flask, request

app = Flask(__name__)

TWILIO_SID   = "ACxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
TWILIO_TOKEN = "your_auth_token"
FROM_NUMBER  = "whatsapp:+14155238886"  # Twilio sandbox number
TO_NUMBER    = "whatsapp:+15551234567"

@app.route('/whatsapp', methods=['POST'])
def relay():
    client = Client(TWILIO_SID, TWILIO_TOKEN)
    events = request.get_json()
    for e in events:
        body = (f"⚠️ Deauth Attack on {e['target_ssid']}\n"
                f"Attacker: {e['attacker_mac']} | Ch: {e['channel']}\n"
                f"Time: {e['timestamp']}")
        client.messages.create(body=body, from_=FROM_NUMBER, to=TO_NUMBER)
    return {"received": len(events)}, 200
```

### Option C — Meta Cloud API (Official)

For production use, the [Meta WhatsApp Business Cloud API](https://developers.facebook.com/docs/whatsapp/cloud-api) provides an official HTTP-based gateway — no Twilio account needed.

```bash
curl -X POST "https://graph.facebook.com/v19.0/PHONE_NUMBER_ID/messages" \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{
    "messaging_product": "whatsapp",
    "to": "15551234567",
    "type": "text",
    "text": { "body": "⚠️ Deauth Attack Detected on Home_WiFi" }
  }'
```

### Pros / Cons

| ✅ Pros | ❌ Cons |
|--------|--------|
| Reaches users on their most-used app | Official API requires Business Account verification |
| CallMeBot option is free with no coding | CallMeBot has rate limits (~20 msg/day) |
| Works on mobile and desktop | Twilio/Meta has per-message cost |

---

## 3. Telegram

Telegram is arguably the easiest notification target for IoT devices. A Telegram Bot can be created in minutes and the HTTP API is simple enough to call directly from the ESP32.

### Option A — Direct from device

The Telegram Bot API is a plain JSON-over-HTTPS call that fits the existing `HTTPClient` pattern.

Create a bot by messaging `@BotFather` in Telegram and get your `BOT_TOKEN`. Then find your `CHAT_ID` by messaging your bot and calling:

```
https://api.telegram.org/bot<BOT_TOKEN>/getUpdates
```

Add a `TelegramReporter` class to the firmware:

```cpp
#include <HTTPClient.h>
#include <ArduinoJson.h>

bool sendTelegramAlert(const String& botToken, const String& chatId,
                       const String& targetSSID, const String& attackerMAC,
                       int channel, const String& timestamp) {
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + botToken + "/sendMessage";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String text = "⚠️ *Deauth Attack Detected*\n"
                  "Network: " + targetSSID + "\n"
                  "Attacker: `" + attackerMAC + "`\n"
                  "Channel: " + String(channel) + "\n"
                  "Time: " + timestamp;

    DynamicJsonDocument doc(512);
    doc["chat_id"]    = chatId;
    doc["text"]       = text;
    doc["parse_mode"] = "Markdown";

    String payload;
    serializeJson(doc, payload);

    int code = http.POST(payload);
    http.end();
    return (code == 200);
}
```

**Config fields to add** (in `APIConfig` or a new `TelegramConfig` struct):

```json
"telegram": {
  "bot_token": "123456789:AAF_your_bot_token_here",
  "chat_id": "-1001234567890"
}
```

### Option B — Via relay

If you prefer to keep all credentials server-side, use the existing webhook and relay to Telegram:

```python
import requests
from flask import Flask, request

app = Flask(__name__)

BOT_TOKEN = "123456789:AAF_your_bot_token"
CHAT_ID   = "-1001234567890"

@app.route('/v1/alerts', methods=['POST'])
def relay():
    events = request.get_json()
    for e in events:
        text = (f"⚠️ *Deauth Attack Detected*\n"
                f"Network: {e['target_ssid']}\n"
                f"Attacker: `{e['attacker_mac']}`\n"
                f"Channel: {e['channel']} | RSSI: {e['rssi']} dBm\n"
                f"Time: {e['timestamp']}")
        requests.post(f"https://api.telegram.org/bot{BOT_TOKEN}/sendMessage",
                      json={"chat_id": CHAT_ID, "text": text, "parse_mode": "Markdown"})
    return {"received": len(events)}, 200
```

### Pros / Cons

| ✅ Pros | ❌ Cons |
|--------|--------|
| 100% free, no rate limits for bots | Requires Telegram app installed |
| Can be called directly from the ESP32 | Bot tokens should be kept off the device ideally |
| Rich formatting (Markdown, inline buttons) | |
| Group/channel support for team alerts | |

---

## 4. SMS via Twilio

Text messages work without a smartphone app and reach virtually any phone worldwide.

### Via relay middleware

```python
from twilio.rest import Client
from flask import Flask, request

app = Flask(__name__)

client = Client("ACxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx", "your_auth_token")
FROM   = "+15551234567"  # your Twilio number
TO     = "+447700900000" # recipient

@app.route('/v1/alerts', methods=['POST'])
def relay():
    events = request.get_json()
    for e in events:
        body = (f"DEAUTH ALERT: Attack on {e['target_ssid']} "
                f"from {e['attacker_mac']} at {e['timestamp']}")
        client.messages.create(body=body, from_=FROM, to=TO)
    return {"received": len(events)}, 200
```

### Alternative SMS Providers

| Provider | Free Tier | Notes |
|----------|-----------|-------|
| Twilio | $15 credit on signup | Most popular, excellent docs |
| Vonage (Nexmo) | €2 credit | Similar API to Twilio |
| AWS SNS | 100 SMS/month (US only) | Cheapest at scale |
| Sinch | Free tier available | Competitive pricing |
| TextBelt | 1 free SMS/day | Simple REST API |

### Pros / Cons

| ✅ Pros | ❌ Cons |
|--------|--------|
| Works on any phone, no app required | Cost per message (typically $0.01–0.05) |
| Very high open rates | 160 character limit per SMS |
| Works in low-connectivity areas | Requires a paid phone number |

---

## 5. ntfy.sh Push Notifications

[ntfy.sh](https://ntfy.sh) is an open-source, self-hostable push notification service with a dead-simple HTTP API. No account is needed for the public instance.

### Option A — Direct from device

```cpp
bool sendNtfyAlert(const String& topic, const String& targetSSID,
                   const String& attackerMAC, int channel) {
    HTTPClient http;
    http.begin("https://ntfy.sh/" + topic);
    http.addHeader("Title", "Deauth Attack Detected");
    http.addHeader("Priority", "urgent");
    http.addHeader("Tags", "warning,wifi");

    String body = "Attack on " + targetSSID + " from " + attackerMAC
                + " (ch " + String(channel) + ")";

    int code = http.POST(body);
    http.end();
    return (code == 200);
}
```

Subscribe to your topic in the ntfy mobile/desktop app or via browser. The topic name (`your-secret-topic-abc123`) acts as a shared secret — choose something random.

### Option B — Self-hosted ntfy

Run your own ntfy server for complete privacy:

```yaml
# docker-compose.yml
version: "3"
services:
  ntfy:
    image: binwiederhier/ntfy
    command: serve
    ports:
      - "8080:80"
    volumes:
      - ntfy-cache:/var/cache/ntfy
      - ntfy-etc:/etc/ntfy
volumes:
  ntfy-cache:
  ntfy-etc:
```

Set the endpoint in `config.txt` to `http://your-server:8080/your-topic`.

### Pros / Cons

| ✅ Pros | ❌ Cons |
|--------|--------|
| Completely free (public instance) | Topic is essentially a shared secret |
| Can be called directly from ESP32 | Public topics are discoverable if not randomised |
| Self-hostable for privacy | App not as mainstream as WhatsApp/Telegram |
| No account required | |

---

## 6. Pushover

[Pushover](https://pushover.net) delivers real-time push notifications to iOS, Android, and desktop. One-time $5 app purchase per platform.

### Via relay (or direct from device)

```python
import requests
from flask import Flask, request

app = Flask(__name__)

PUSHOVER_TOKEN = "azGDORePK8gMaC0QOYAMyEEuzJnyUi"
PUSHOVER_USER  = "uQiRzpo4DXghDmr9QzzfQu"

@app.route('/v1/alerts', methods=['POST'])
def relay():
    events = request.get_json()
    for e in events:
        requests.post("https://api.pushover.net/1/messages.json", data={
            "token":    PUSHOVER_TOKEN,
            "user":     PUSHOVER_USER,
            "title":    f"⚠️ Deauth Attack on {e['target_ssid']}",
            "message":  (f"Attacker: {e['attacker_mac']}\n"
                         f"Channel: {e['channel']} | {e['timestamp']}"),
            "priority": 1,  # high priority
            "sound":    "siren"
        })
    return {"received": len(events)}, 200
```

### Pros / Cons

| ✅ Pros | ❌ Cons |
|--------|--------|
| Reliable delivery with receipts | One-time $5 app cost |
| Emergency priority bypasses Do-Not-Disturb | 10,000 msg/month free then $5/month |
| Custom sounds and priorities | |

---

## 7. MQTT Broker

[MQTT](https://mqtt.org) is a lightweight publish/subscribe protocol widely used in IoT. Events published to a broker topic can be consumed by any subscriber — including Node-RED flows, Home Assistant, and custom apps.

### Option A — Direct from device (PubSubClient library)

```ini
# platformio.ini
lib_deps =
    knolleary/PubSubClient@^2.8
```

```cpp
#include <PubSubClient.h>
#include <WiFiClient.h>

WiFiClient   wifiClient;
PubSubClient mqttClient(wifiClient);

void connectMQTT() {
    mqttClient.setServer("broker.hivemq.com", 1883); // or your broker
    while (!mqttClient.connected()) {
        mqttClient.connect("deauth-detector");
    }
}

void publishAlert(const String& targetSSID, const String& attackerMAC,
                  int channel, const String& timestamp) {
    if (!mqttClient.connected()) connectMQTT();

    DynamicJsonDocument doc(256);
    doc["target_ssid"]  = targetSSID;
    doc["attacker_mac"] = attackerMAC;
    doc["channel"]      = channel;
    doc["timestamp"]    = timestamp;

    String payload;
    serializeJson(doc, payload);
    mqttClient.publish("deauth-detector/alerts", payload.c_str());
}
```

### Node-RED Flow (subscriber example)

1. Add an `mqtt in` node subscribed to `deauth-detector/alerts`
2. Parse the JSON payload
3. Route to any of the other notification channels (email, Telegram, etc.)

### Home Assistant MQTT Sensor

```yaml
# configuration.yaml
mqtt:
  sensor:
    - name: "Deauth Detector Last Alert"
      state_topic: "deauth-detector/alerts"
      value_template: "{{ value_json.target_ssid }}"
```

### Pros / Cons

| ✅ Pros | ❌ Cons |
|--------|--------|
| Ultra-lightweight (ideal for ESP32) | No built-in notification delivery |
| Decouples the device from notification logic | Requires a broker (cloud or self-hosted) |
| Integrates with Node-RED, HA, Grafana | |
| Fan-out to many subscribers simultaneously | |

---

## 8. PagerDuty

[PagerDuty](https://www.pagerduty.com) is an enterprise incident management platform with an Events API v2 designed for exactly this kind of security alert.

### Via relay

```python
import requests
from flask import Flask, request

app = Flask(__name__)

PAGERDUTY_ROUTING_KEY = "R015xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

@app.route('/v1/alerts', methods=['POST'])
def relay():
    events = request.get_json()
    for e in events:
        requests.post("https://events.pagerduty.com/v2/enqueue",
            headers={"Content-Type": "application/json"},
            json={
                "routing_key":  PAGERDUTY_ROUTING_KEY,
                "event_action": "trigger",
                "payload": {
                    "summary":   f"Deauth attack on {e['target_ssid']}",
                    "severity":  "critical",
                    "source":    "M5 Deauth Detector",
                    "timestamp": e['timestamp'],
                    "custom_details": e
                }
            })
    return {"received": len(events)}, 200
```

### Pros / Cons

| ✅ Pros | ❌ Cons |
|--------|--------|
| Enterprise-grade on-call scheduling | Overkill for personal use |
| Escalation policies and acknowledgement | Free tier limited to 5 users |
| Integrates with many monitoring tools | |

---

## 9. AWS SNS / SES

Amazon Web Services offers two complementary notification services:

- **SNS (Simple Notification Service)** — Fanout to SMS, email, Lambda, SQS, HTTP
- **SES (Simple Email Service)** — High-volume, high-deliverability transactional email

### Via relay (Lambda or Python)

```python
import boto3, json
from flask import Flask, request

app = Flask(__name__)

sns = boto3.client('sns', region_name='us-east-1')
TOPIC_ARN = "arn:aws:sns:us-east-1:123456789012:DeauthAlerts"

@app.route('/v1/alerts', methods=['POST'])
def relay():
    events = request.get_json()
    for e in events:
        sns.publish(
            TopicArn=TOPIC_ARN,
            Subject=f"Deauth Attack on {e['target_ssid']}",
            Message=json.dumps(e, indent=2)
        )
    return {"received": len(events)}, 200
```

By subscribing to the SNS topic you can fan out to email, SMS, Lambda functions, and HTTP endpoints simultaneously.

### Pros / Cons

| ✅ Pros | ❌ Cons |
|--------|--------|
| Fan-out to multiple channels at once | Requires AWS account and IAM setup |
| Scales to any volume | More complex to set up |
| SMS + email + Lambda from one publish | Cost at scale (very cheap, but not free) |

---

## 10. Gotify (Self-hosted Push)

[Gotify](https://gotify.net) is a free, self-hosted alternative to Pushover. Run it on a Raspberry Pi or VPS.

### Setup

```yaml
# docker-compose.yml
version: "3"
services:
  gotify:
    image: gotify/server
    ports:
      - "8080:80"
    volumes:
      - gotify-data:/app/data
    environment:
      - GOTIFY_DEFAULTUSER_PASS=admin
volumes:
  gotify-data:
```

### Direct from device

```cpp
bool sendGotifyAlert(const String& serverUrl, const String& appToken,
                     const String& targetSSID, const String& attackerMAC) {
    HTTPClient http;
    http.begin(serverUrl + "/message?token=" + appToken);
    http.addHeader("Content-Type", "application/json");

    DynamicJsonDocument doc(256);
    doc["title"]    = "Deauth Attack Detected";
    doc["message"]  = "Attack on " + targetSSID + " from " + attackerMAC;
    doc["priority"] = 8;

    String payload;
    serializeJson(doc, payload);
    int code = http.POST(payload);
    http.end();
    return (code == 200);
}
```

### Pros / Cons

| ✅ Pros | ❌ Cons |
|--------|--------|
| 100% free and self-hosted | Requires your own server |
| Can be called directly from ESP32 | Server must be reachable from device network |
| Android + web app available | iOS app is community-maintained |

---

## Comparison Summary

| Method | Difficulty | Cost | Direct from ESP32 | Requires App | Best For |
|--------|-----------|------|:-----------------:|:------------:|---------|
| **SMTP Email** | Medium | Free–low | ✅ (with library) | No | Universal delivery |
| **WhatsApp (CallMeBot)** | Easy | Free | Via relay | ✅ WhatsApp | Personal/home use |
| **WhatsApp (Twilio)** | Medium | ~$0.005/msg | Via relay | ✅ WhatsApp | Business use |
| **Telegram Bot** | Easy | Free | ✅ Direct | ✅ Telegram | Hobbyists / teams |
| **SMS (Twilio)** | Medium | ~$0.01/msg | Via relay | No | Critical alerts |
| **ntfy.sh** | Easy | Free | ✅ Direct | ✅ ntfy | Privacy-conscious |
| **Pushover** | Easy | $5 one-time | Via relay | ✅ Pushover | Reliable push |
| **MQTT** | Medium | Free | ✅ Direct | No | IoT / Node-RED |
| **PagerDuty** | Complex | Free–paid | Via relay | No | Enterprise / on-call |
| **AWS SNS/SES** | Complex | Free–low | Via relay | No | Fan-out at scale |
| **Gotify** | Medium | Free | ✅ Direct | ✅ Gotify | Self-hosted push |

---

## Recommended Approaches

### For personal / home use
1. **Telegram Bot** — Free, direct from ESP32, excellent UX, no cost
2. **ntfy.sh** — Zero friction, no account needed, self-hostable
3. **CallMeBot WhatsApp** — Best if you primarily use WhatsApp

### For small business / team
1. **Telegram group/channel** — Fan-out to entire team for free
2. **SMTP email** — Familiar, archives automatically, works everywhere
3. **MQTT + Node-RED** — Flexible pipeline; route alerts wherever you need

### For enterprise
1. **PagerDuty** — On-call scheduling, escalation, acknowledgement
2. **AWS SNS** — Fan-out to email + SMS + Slack + custom Lambda simultaneously
3. **SMTP via SendGrid/SES** — High-volume, auditable delivery

---

## Implementation Path

Regardless of which channel you choose, the integration pattern is identical:

```
ESP32 Device
    │
    │  HTTP POST (existing API webhook)
    ▼
Relay Server / Cloud Function
    │
    ├──► Email (SMTP / SendGrid / SES)
    ├──► WhatsApp (CallMeBot / Twilio / Meta)
    ├──► Telegram Bot API
    ├──► SMS (Twilio / Vonage)
    ├──► Push (ntfy / Pushover / Gotify)
    ├──► MQTT Broker
    └──► PagerDuty / AWS SNS / ...
```

No firmware changes are needed for relay-based integrations — only the `endpoint_url` in `config.txt` needs to be updated to point at your relay server.

For **direct** integrations (Telegram, ntfy, Gotify, SMTP, MQTT), a new reporter class would be added to the firmware following the same pattern as `APIReporter`.

---

## Related Documentation

- [API Integration](api-integration.md) — Existing REST API webhook format and payload
- [Configuration Reference](configuration.md) — All settings explained
- [Troubleshooting](troubleshooting.md) — Connectivity and reporting issues
