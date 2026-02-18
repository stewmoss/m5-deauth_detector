# API Integration

This guide covers integrating the M5 Cardputer Deauth Detector with external security systems via the REST API reporting feature.

---

## Overview

The device can send detection events to an external HTTP/HTTPS endpoint in real-time. Events are batched and transmitted at configurable intervals, enabling integration with:

- Security Information and Event Management (SIEM) systems
- Custom alerting platforms
- Logging aggregators
- Webhook endpoints (Slack, Discord, Teams, etc.)
- Home automation systems

---

## Configuration

### Required Settings

Configure these in the **API** tab of the web interface or directly in `config.txt`:

```json
"api": {
  "endpoint_url": "https://your-api.com/v1/alerts",
  "custom_header_name": "X-API-KEY",
  "custom_header_value": "your-secret-key"
}
```

| Parameter | Description |
|-----------|-------------|
| `endpoint_url` | Full URL for HTTP POST requests |
| `custom_header_name` | Header name for authentication |
| `custom_header_value` | Header value for authentication |

### Reporting Interval

Set in the **Detection** tab:

```json
"detection": {
  "reporting_interval_seconds": 10
}
```

This controls how frequently the device connects to WiFi and sends batched events.

---

## HTTP Request Format

### Request Details

| Property | Value |
|----------|-------|
| **Method** | POST |
| **Content-Type** | application/json |
| **Custom Header** | As configured (e.g., `X-API-KEY: your-key`) |

### Example Request

```http
POST /v1/alerts HTTP/1.1
Host: your-api.com
Content-Type: application/json
X-API-KEY: your-secret-key

[
  {
    "timestamp": "2026-01-30T14:20:01Z",
    "target_ssid": "Home_WiFi",
    "target_bssid": "AA:BB:CC:DD:EE:FF",
    "attacker_mac": "11:22:33:44:55:66",
    "channel": 6,
    "rssi": -55,
    "packet_count": 24
  }
]
```

---

## Payload Schema

Events are sent as a JSON array of objects:

```json
[
  {
    "timestamp": "string (ISO 8601)",
    "target_ssid": "string",
    "target_bssid": "string (MAC address)",
    "attacker_mac": "string (MAC address)",
    "channel": "integer (1-14)",
    "rssi": "integer (dBm, negative)",
    "packet_count": "integer"
  }
]
```

### Field Descriptions

| Field | Type | Description |
|-------|------|-------------|
| `timestamp` | String | ISO 8601 UTC timestamp (e.g., `2026-01-30T14:20:01Z`) |
| `target_ssid` | String | Name of the network being attacked |
| `target_bssid` | String | MAC address of the access point (format: `AA:BB:CC:DD:EE:FF`) |
| `attacker_mac` | String | Source MAC of deauth frame, or `"Unknown"` if not determinable |
| `channel` | Integer | Wi-Fi channel (1-14) where attack occurred |
| `rssi` | Integer | Signal strength in dBm (typically -30 to -90) |
| `packet_count` | Integer | Number of deauth packets in this event |

### Example Payloads

**Single Event:**

```json
[
  {
    "timestamp": "2026-01-30T14:20:01Z",
    "target_ssid": "Home_WiFi",
    "target_bssid": "AA:BB:CC:DD:EE:FF",
    "attacker_mac": "11:22:33:44:55:66",
    "channel": 6,
    "rssi": -55,
    "packet_count": 24
  }
]
```

**Multiple Events (Batch):**

```json
[
  {
    "timestamp": "2026-01-30T14:20:01Z",
    "target_ssid": "Home_WiFi",
    "target_bssid": "AA:BB:CC:DD:EE:FF",
    "attacker_mac": "11:22:33:44:55:66",
    "channel": 6,
    "rssi": -55,
    "packet_count": 24
  },
  {
    "timestamp": "2026-01-30T14:20:08Z",
    "target_ssid": "Home_WiFi",
    "target_bssid": "AA:BB:CC:DD:EE:FF",
    "attacker_mac": "11:22:33:44:55:66",
    "channel": 6,
    "rssi": -52,
    "packet_count": 18
  },
  {
    "timestamp": "2026-01-30T14:22:15Z",
    "target_ssid": "Office_Secure",
    "target_bssid": "DD:EE:FF:AA:BB:CC",
    "attacker_mac": "Unknown",
    "channel": 11,
    "rssi": -68,
    "packet_count": 6
  }
]
```

---

## Authentication Methods

### API Key Header

Most common method for API authentication:

```json
"api": {
  "endpoint_url": "https://api.example.com/alerts",
  "custom_header_name": "X-API-KEY",
  "custom_header_value": "sk_live_abc123xyz"
}
```

### Bearer Token

For OAuth2/JWT-based systems:

```json
"api": {
  "endpoint_url": "https://api.example.com/alerts",
  "custom_header_name": "Authorization",
  "custom_header_value": "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..."
}
```

### Basic Authentication

For HTTP Basic Auth (base64-encoded credentials):

```json
"api": {
  "endpoint_url": "https://api.example.com/alerts",
  "custom_header_name": "Authorization",
  "custom_header_value": "Basic dXNlcm5hbWU6cGFzc3dvcmQ="
}
```

To generate Basic Auth value: `base64("username:password")`

### No Authentication

For internal/trusted endpoints:

```json
"api": {
  "endpoint_url": "http://192.168.1.100:8080/webhook",
  "custom_header_name": "",
  "custom_header_value": ""
}
```

---

## Integration Examples

### Webhook Server (Node.js)

Simple Express server to receive alerts:

```javascript
const express = require('express');
const app = express();

app.use(express.json());

app.post('/v1/alerts', (req, res) => {
  const apiKey = req.headers['x-api-key'];
  
  if (apiKey !== 'your-secret-key') {
    return res.status(401).json({ error: 'Unauthorized' });
  }
  
  const events = req.body;
  
  events.forEach(event => {
    console.log(`[${event.timestamp}] Attack on ${event.target_ssid}`);
    console.log(`  Attacker: ${event.attacker_mac}`);
    console.log(`  Channel: ${event.channel}, RSSI: ${event.rssi}`);
    console.log(`  Packets: ${event.packet_count}`);
  });
  
  res.status(200).json({ received: events.length });
});

app.listen(3000, () => {
  console.log('Alert server running on port 3000');
});
```

### Slack Webhook

To send alerts to Slack, you'll need a middleware to transform the payload:

```javascript
// Middleware example
app.post('/slack-relay', async (req, res) => {
  const events = req.body;
  
  for (const event of events) {
    await fetch(process.env.SLACK_WEBHOOK_URL, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        text: `🚨 *Deauth Attack Detected*`,
        blocks: [
          {
            type: 'section',
            text: {
              type: 'mrkdwn',
              text: `*Network:* ${event.target_ssid}\n*Attacker:* \`${event.attacker_mac}\`\n*Channel:* ${event.channel} | *RSSI:* ${event.rssi} dBm\n*Time:* ${event.timestamp}`
            }
          }
        ]
      })
    });
  }
  
  res.status(200).json({ ok: true });
});
```

### Python Flask Server

```python
from flask import Flask, request, jsonify
from datetime import datetime

app = Flask(__name__)

@app.route('/v1/alerts', methods=['POST'])
def receive_alerts():
    api_key = request.headers.get('X-API-KEY')
    
    if api_key != 'your-secret-key':
        return jsonify({'error': 'Unauthorized'}), 401
    
    events = request.get_json()
    
    for event in events:
        print(f"[{event['timestamp']}] Attack detected!")
        print(f"  Target: {event['target_ssid']} ({event['target_bssid']})")
        print(f"  Attacker: {event['attacker_mac']}")
        print(f"  Channel: {event['channel']}, RSSI: {event['rssi']} dBm")
        print(f"  Packet count: {event['packet_count']}")
        
        # Store in database, send notifications, etc.
    
    return jsonify({'received': len(events)}), 200

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
```

### Home Assistant Webhook

Configure in `config.txt`:

```json
"api": {
  "endpoint_url": "http://homeassistant.local:8123/api/webhook/deauth_alert",
  "custom_header_name": "Authorization",
  "custom_header_value": "Bearer YOUR_LONG_LIVED_TOKEN"
}
```

Home Assistant automation example:

```yaml
automation:
  - alias: "Deauth Attack Alert"
    trigger:
      - platform: webhook
        webhook_id: deauth_alert
    action:
      - service: notify.mobile_app
        data:
          title: "WiFi Attack Detected!"
          message: "Deauthentication attack on {{ trigger.json[0].target_ssid }}"
```

---

## Error Handling

### Device Behavior on Failure

| Scenario | Device Response |
|----------|-----------------|
| Network unreachable | Events remain queued, retry next interval |
| HTTP 4xx response | Events discarded (invalid request) |
| HTTP 5xx response | Events remain queued for retry |
| Timeout | Events remain queued for retry |

### Recommended Server Responses

| Status Code | Meaning | Device Behavior |
|-------------|---------|-----------------|
| 200 | Success | Clear event queue |
| 201 | Created | Clear event queue |
| 204 | No Content | Clear event queue |
| 400 | Bad Request | Discard events |
| 401 | Unauthorized | Discard events |
| 500 | Server Error | Retry next interval |
| 503 | Unavailable | Retry next interval |

---

## Security Recommendations

### Transport Security

- **Use HTTPS** for production deployments
- The ESP32 supports TLS 1.2
- Self-signed certificates may require additional configuration

### API Key Management

- Use unique keys per device for tracking
- Rotate keys periodically
- Store keys securely (environment variables, secrets manager)

### Network Isolation

- Consider a dedicated VLAN for security devices
- Firewall rules to limit API access
- VPN for remote endpoints

### Rate Limiting

- Implement rate limiting on your endpoint
- Typical: 100 requests/minute per device
- Monitor for unusual traffic patterns

---

## Testing the Integration

### Using curl

Test your endpoint before configuring the device:

```bash
curl -X POST https://your-api.com/v1/alerts \
  -H "Content-Type: application/json" \
  -H "X-API-KEY: your-secret-key" \
  -d '[{
    "timestamp": "2026-01-30T12:00:00Z",
    "target_ssid": "TestNetwork",
    "target_bssid": "AA:BB:CC:DD:EE:FF",
    "attacker_mac": "11:22:33:44:55:66",
    "channel": 6,
    "rssi": -50,
    "packet_count": 10
  }]'
```

### Debug Logging

Enable debug logging to see API activity:

1. Enable debug in config
2. View debug log via web interface
3. Look for "API POST" messages

### Local Testing

For initial testing, use a local endpoint:

```json
"api": {
  "endpoint_url": "http://192.168.1.100:3000/test",
  "custom_header_name": "",
  "custom_header_value": ""
}
```

---

## Disabling API Reporting

To disable remote reporting entirely, clear the endpoint URL:

```json
"api": {
  "endpoint_url": "",
  "custom_header_name": "",
  "custom_header_value": ""
}
```

Events will still be logged locally to the SD card.

---

## Related Documentation

- [Configuration Reference](configuration.md) — All settings explained
- [Operation Guide](operation.md) — Understanding detection events
- [Troubleshooting](troubleshooting.md) — API-related issues
