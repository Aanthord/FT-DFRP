const WebSocket = require('ws');
const express = require('express');
const path = require('path');
const fs = require('fs');

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
        
        // Disable X-Powered-By header
        this.app.disable('x-powered-by');
        
        // Add explicit MIME types
        this.app.use((req, res, next) => {
            const ext = path.extname(req.url);
            if (ext === '.html') {
                res.type('text/html');
            } else if (ext === '.js') {
                res.type('application/javascript');
            } else if (ext === '.css') {
                res.type('text/css');
            } else if (ext === '.json') {
                res.type('application/json');
            }
            next();
        });
        
        // Serve static files from current directory
        this.app.use(express.static('.', {
            index: false, // Don't auto-serve index.html
            setHeaders: (res, filePath) => {
                // Disable caching for demo files
                if (filePath.endsWith('.html')) {
                    res.set('Cache-Control', 'no-cache, no-store, must-revalidate');
                }
            }
        }));
        
        // Explicit routes for demo files
        this.app.get('/', (req, res) => {
            res.send(this.getServerHomePage());
        });
        
        this.app.get('/ft-dfrp-simple.html', (req, res) => {
            this.serveFileIfExists(res, 'ft-dfrp-simple.html');
        });
        
        this.app.get('/ft-dfrp-crossdevice-demo.html', (req, res) => {
            this.serveFileIfExists(res, 'ft-dfrp-crossdevice-demo.html');
        });
        
        this.app.get('/browser-diagnostic.html', (req, res) => {
            this.serveFileIfExists(res, 'browser-diagnostic.html');
        });
        
        this.app.get('/ft-dfrp-live-demo.html', (req, res) => {
            this.serveFileIfExists(res, 'ft-dfrp-live-demo.html');
        });
        
        // Catch-all for missing files
        this.app.get('*', (req, res) => {
            res.status(404).send(`
                <html>
                <head><title>FT-DFRP Server - File Not Found</title></head>
                <body style="font-family: monospace; padding: 20px; background: #000; color: #0f0;">
                    <h1>FT-DFRP Signaling Server</h1>
                    <p><strong>File not found:</strong> ${req.url}</p>
                    <h2>Available Files:</h2>
                    <ul>
                        <li><a href="/ft-dfrp-simple.html">ft-dfrp-simple.html</a> (Recommended - works with older browsers)</li>
                        <li><a href="/ft-dfrp-crossdevice-demo.html">ft-dfrp-crossdevice-demo.html</a> (Full version)</li>
                        <li><a href="/browser-diagnostic.html">browser-diagnostic.html</a> (Compatibility check)</li>
                        <li><a href="/ft-dfrp-live-demo.html">ft-dfrp-live-demo.html</a> (Same-browser tabs)</li>
                    </ul>
                    <p><strong>Server Status:</strong> WebSocket signaling active on port ${this.port}</p>
                </body>
                </html>
            `);
        });
        
        // Start HTTP server
        this.server = this.app.listen(this.port, () => {
            console.log(`FT-DFRP HTTP Server: http://localhost:${this.port}`);
            this.printNetworkInfo();
        });
        
        // WebSocket server for signaling
        this.wss = new WebSocket.Server({ server: this.server });
        this.wss.on('connection', (ws, req) => {
            this.handleNewConnection(ws, req);
        });

        console.log(`FT-DFRP Signaling Server started on port ${this.port}`);
        console.log(`\n🌐 DEMO URLS:`);
        console.log(`   http://localhost:${this.port}/ft-dfrp-simple.html (Recommended)`);
        console.log(`   http://localhost:${this.port}/browser-diagnostic.html (Check compatibility)`);
    }
    
    serveFileIfExists(res, filename) {
        if (fs.existsSync(filename)) {
            res.sendFile(path.resolve(filename));
        } else {
            res.status(404).send(`
                <html>
                <head><title>FT-DFRP - File Missing</title></head>
                <body style="font-family: monospace; padding: 20px; background: #000; color: #f00;">
                    <h1>File Missing: ${filename}</h1>
                    <p>Please download the complete FT-DFRP package including:</p>
                    <ul>
                        <li>ft-dfrp-simple.html</li>
                        <li>ft-dfrp-crossdevice-demo.html</li>
                        <li>browser-diagnostic.html</li>
                        <li>signaling-server.js</li>
                    </ul>
                    <p><a href="/">← Back to server home</a></p>
                </body>
                </html>
            `);
        }
    }
    
    getServerHomePage() {
        return `
            <html>
            <head>
                <title>FT-DFRP Signaling Server</title>
                <style>
                    body { font-family: monospace; padding: 20px; background: #000; color: #0f0; }
                    a { color: #00ff88; }
                    .status { background: rgba(0,255,136,0.1); padding: 15px; border: 1px solid #00ff88; border-radius: 5px; margin: 10px 0; }
                </style>
            </head>
            <body>
                <h1>🌐 FT-DFRP Signaling Server Active</h1>
                
                <div class="status">
                    <strong>Server Status:</strong> ✅ Running on port ${this.port}<br>
                    <strong>WebSocket:</strong> ✅ Signaling active<br>
                    <strong>Connected Clients:</strong> ${this.clients.size}
                </div>
                
                <h2>🚀 Demo Links</h2>
                <ul>
                    <li><a href="/ft-dfrp-simple.html"><strong>ft-dfrp-simple.html</strong></a> - Simple cross-device demo (works with any browser)</li>
                    <li><a href="/browser-diagnostic.html">browser-diagnostic.html</a> - Check browser compatibility</li>
                    <li><a href="/ft-dfrp-crossdevice-demo.html">ft-dfrp-crossdevice-demo.html</a> - Full-featured demo</li>
                    <li><a href="/ft-dfrp-live-demo.html">ft-dfrp-live-demo.html</a> - Same-browser tabs only</li>
                </ul>
                
                <h2>📱 Cross-Device Instructions</h2>
                <p>1. Open <strong>ft-dfrp-simple.html</strong> on your PC</p>
                <p>2. Open the same URL on your Mac</p>
                <p>3. Watch nodes discover each other and form mesh network!</p>
                
                <div class="status">
                    <strong>For Your Setup:</strong><br>
                    PC (node 135): Open demo on this server<br>
                    Mac (node 195): Open same URL from Mac browser<br>
                    Expected: Cross-device mesh formation via WebSocket signaling
                </div>
            </body>
            </html>
        `;
    }
    
    printNetworkInfo() {
        const networkInterfaces = require('os').networkInterfaces();
        console.log(`\n📱 NETWORK ACCESS:`);
        
        Object.keys(networkInterfaces).forEach(interfaceName => {
            const interfaces = networkInterfaces[interfaceName];
            interfaces.forEach(iface => {
                if (iface.family === 'IPv4' && !iface.internal) {
                    console.log(`   http://${iface.address}:${this.port}/ft-dfrp-simple.html`);
                }
            });
        });
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
        console.log(`WebSocket client connected: ${clientId} from ${clientInfo.ip}`);

        ws.on('message', (data) => {
            this.handleMessage(clientId, data);
        });

        ws.on('close', () => {
            console.log(`WebSocket client disconnected: ${clientId}`);
            this.clients.delete(clientId);
        });

        ws.on('error', (error) => {
            console.log(`WebSocket client error ${clientId}:`, error.message);
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
                    console.log(`Presence announcement from ${message.nodeId?.slice(-8) || clientId}`);
                    this.broadcastToOthers(clientId, message);
                    break;
                case 'offer':
                case 'answer':
                case 'ice-candidate':
                    console.log(`WebRTC signaling: ${message.type} from ${message.nodeId?.slice(-8)} to ${message.targetId?.slice(-8)}`);
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
    const server = new FTDFRPSignalingServer(port);
    
    // Store globally for stats
    global.signalingServer = server;

    // Log periodic stats
    setInterval(() => {
        const stats = server.getStats();
        if (stats.connectedClients > 0) {
            console.log(`\n📊 Stats: ${stats.connectedClients} WebSocket clients, ${stats.nodes.length} identified nodes`);
            stats.nodes.forEach(node => {
                console.log(`   Node ${node.nodeId} from ${node.ip}`);
            });
        }
    }, 30000);
}

module.exports = FTDFRPSignalingServer;
