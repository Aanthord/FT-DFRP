const WebSocket = require('ws');
const express = require('express');
const path = require('path');

// Simple signaling server for FT-DFRP cross-device networking
class FTDFRPSignalingServer {
    constructor(port = 8080) {
        this.port = port;
        this.clients = new Map();
        this.setupServer();
    }

    setupServer() {
        // Express server for serving static files
        this.app = express();
        this.app.use(express.static('.'));
        
        // WebSocket server for signaling
        this.wss = new WebSocket.Server({ port: this.port });
        this.wss.on('connection', (ws, req) => {
            this.handleNewConnection(ws, req);
        });

        console.log(`FT-DFRP Signaling Server started on port ${this.port}`);
        console.log(`Web interface: http://localhost:${this.port}/ft-dfrp-crossdevice-demo.html`);
        console.log(`Network access: http://[YOUR-IP]:${this.port}/ft-dfrp-crossdevice-demo.html`);
    }

    handleNewConnection(ws, req) {
        const clientId = this.generateClientId();
        const clientInfo = {
            id: clientId,
            ws: ws,
            nodeId: null,
            lastSeen: Date.now(),
            ip: req.socket.remoteAddress
        };

        this.clients.set(clientId, clientInfo);
        console.log(`Client connected: ${clientId} from ${clientInfo.ip}`);

        ws.on('message', (data) => {
            this.handleMessage(clientId, data);
        });

        ws.on('close', () => {
            console.log(`Client disconnected: ${clientId}`);
            this.clients.delete(clientId);
        });

        ws.on('error', (error) => {
            console.log(`Client error ${clientId}:`, error.message);
            this.clients.delete(clientId);
        });
    }

    handleMessage(clientId, data) {
        try {
            const message = JSON.parse(data.toString());
            const client = this.clients.get(clientId);
            
            if (!client) return;

            // Update client node ID on first message
            if (message.nodeId && !client.nodeId) {
                client.nodeId = message.nodeId;
                console.log(`Client ${clientId} identified as node: ${message.nodeId.slice(-8)}`);
            }

            client.lastSeen = Date.now();

            // Route message based on type
            switch (message.type) {
                case 'presence':
                    this.broadcastToOthers(clientId, message);
                    break;
                case 'offer':
                case 'answer':
                case 'ice-candidate':
                    this.routeToTarget(message.targetId, message);
                    break;
                default:
                    this.broadcastToOthers(clientId, message);
            }
        } catch (error) {
            console.log(`Invalid message from ${clientId}:`, error.message);
        }
    }

    broadcastToOthers(senderClientId, message) {
        this.clients.forEach((client, clientId) => {
            if (clientId !== senderClientId && client.ws.readyState === WebSocket.OPEN) {
                try {
                    client.ws.send(JSON.stringify(message));
                } catch (error) {
                    console.log(`Failed to send to ${clientId}:`, error.message);
                }
            }
        });
    }

    routeToTarget(targetNodeId, message) {
        // Find client with matching node ID
        let targetClient = null;
        this.clients.forEach((client) => {
            if (client.nodeId === targetNodeId) {
                targetClient = client;
            }
        });

        if (targetClient && targetClient.ws.readyState === WebSocket.OPEN) {
            try {
                targetClient.ws.send(JSON.stringify(message));
            } catch (error) {
                console.log(`Failed to route to ${targetNodeId}:`, error.message);
            }
        }
    }

    generateClientId() {
        return 'client_' + Math.random().toString(36).substr(2, 9);
    }

    getStats() {
        return {
            connectedClients: this.clients.size,
            nodes: Array.from(this.clients.values())
                .filter(c => c.nodeId)
                .map(c => ({
                    nodeId: c.nodeId.slice(-8),
                    ip: c.ip,
                    lastSeen: c.lastSeen
                }))
        };
    }
}

// Start server if run directly
if (require.main === module) {
    const port = process.argv[2] || 8080;
    new FTDFRPSignalingServer(port);

    // Log periodic stats
    setInterval(() => {
        const server = global.signalingServer;
        if (server) {
            const stats = server.getStats();
            console.log(`Stats: ${stats.connectedClients} clients, ${stats.nodes.length} identified nodes`);
        }
    }, 30000);
}

module.exports = FTDFRPSignalingServer;
