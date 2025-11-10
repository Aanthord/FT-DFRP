// FT-DFRP Service Worker for Background P2P Networking
// Handles persistent connections and background mesh management

const CACHE_NAME = 'ft-dfrp-v1';
const SW_VERSION = '1.0.0';

// Install service worker
self.addEventListener('install', (event) => {
    console.log('[FT-DFRP SW] Installing service worker v' + SW_VERSION);
    event.waitUntil(
        caches.open(CACHE_NAME).then((cache) => {
            return cache.addAll([
                './',
                './ft-dfrp-live-demo.html',
                './index.html'
            ]);
        })
    );
    self.skipWaiting();
});

// Activate service worker
self.addEventListener('activate', (event) => {
    console.log('[FT-DFRP SW] Activating service worker');
    event.waitUntil(
        caches.keys().then((cacheNames) => {
            return Promise.all(
                cacheNames.map((cacheName) => {
                    if (cacheName !== CACHE_NAME) {
                        console.log('[FT-DFRP SW] Deleting old cache:', cacheName);
                        return caches.delete(cacheName);
                    }
                })
            );
        })
    );
    self.clients.claim();
});

// Handle fetch requests
self.addEventListener('fetch', (event) => {
    if (event.request.url.includes('chrome-extension://') || 
        event.request.url.includes('moz-extension://')) {
        return; // Skip extension requests
    }

    event.respondWith(
        caches.match(event.request).then((response) => {
            if (response) {
                return response;
            }
            return fetch(event.request);
        })
    );
});

// Background P2P network management
let networkState = {
    nodeId: null,
    peers: new Map(),
    lastAnnouncement: 0,
    backgroundConnections: new Set()
};

// Handle messages from main thread
self.addEventListener('message', (event) => {
    const { type, data } = event.data;
    
    switch (type) {
        case 'init-network':
            initializeBackgroundNetwork(data);
            break;
        case 'peer-discovered':
            handleBackgroundPeerDiscovery(data);
            break;
        case 'network-sync':
            syncNetworkState(data);
            break;
        case 'ping-peers':
            performBackgroundPing();
            break;
    }
});

function initializeBackgroundNetwork(config) {
    networkState.nodeId = config.nodeId;
    console.log(`[FT-DFRP SW] Initialized background network for node: ${config.nodeId.slice(-8)}`);
    
    // Start periodic background tasks
    setInterval(performBackgroundMaintenance, 10000); // Every 10 seconds
    setInterval(broadcastPresence, 30000); // Every 30 seconds
}

function handleBackgroundPeerDiscovery(peerData) {
    if (!networkState.peers.has(peerData.nodeId)) {
        networkState.peers.set(peerData.nodeId, {
            ...peerData,
            discoveredAt: Date.now(),
            lastSeen: Date.now(),
            connectionAttempts: 0
        });
        
        console.log(`[FT-DFRP SW] Background discovery: ${peerData.nodeId.slice(-8)}`);
        
        // Notify main thread
        notifyMainThread('peer-discovered', peerData);
    }
}

function syncNetworkState(stateData) {
    // Merge remote state with local background state
    if (stateData.peers) {
        Object.entries(stateData.peers).forEach(([peerId, peerData]) => {
            if (peerId !== networkState.nodeId) {
                networkState.peers.set(peerId, {
                    ...peerData,
                    lastSeen: Date.now()
                });
            }
        });
    }
}

function performBackgroundMaintenance() {
    const now = Date.now();
    const staleThreshold = 60000; // 1 minute
    
    // Clean up stale peers
    networkState.peers.forEach((peer, peerId) => {
        if (now - peer.lastSeen > staleThreshold) {
            console.log(`[FT-DFRP SW] Removing stale peer: ${peerId.slice(-8)}`);
            networkState.peers.delete(peerId);
            notifyMainThread('peer-removed', { nodeId: peerId });
        }
    });
    
    // Report network health
    notifyMainThread('network-health', {
        activePeers: networkState.peers.size,
        nodeId: networkState.nodeId,
        timestamp: now
    });
}

function broadcastPresence() {
    if (networkState.nodeId) {
        const presenceData = {
            nodeId: networkState.nodeId,
            timestamp: Date.now(),
            swVersion: SW_VERSION,
            peerCount: networkState.peers.size
        };
        
        notifyMainThread('broadcast-presence', presenceData);
        networkState.lastAnnouncement = Date.now();
    }
}

function performBackgroundPing() {
    // Send ping to all known peers through main thread
    networkState.peers.forEach((peer, peerId) => {
        notifyMainThread('ping-peer', { targetId: peerId });
    });
}

function notifyMainThread(type, data) {
    // Send message to all connected clients
    self.clients.matchAll({ type: 'window' }).then((clients) => {
        clients.forEach((client) => {
            client.postMessage({
                type: 'sw-message',
                messageType: type,
                data: data,
                timestamp: Date.now()
            });
        });
    });
}

// Handle background sync for mesh resilience
self.addEventListener('sync', (event) => {
    if (event.tag === 'network-maintenance') {
        event.waitUntil(performBackgroundMaintenance());
    } else if (event.tag === 'peer-discovery') {
        event.waitUntil(broadcastPresence());
    }
});

// Handle push notifications for network events (if enabled)
self.addEventListener('push', (event) => {
    if (event.data) {
        try {
            const data = event.data.json();
            
            if (data.type === 'network-event') {
                const notificationOptions = {
                    body: `Network event: ${data.message}`,
                    icon: '/favicon.ico',
                    badge: '/favicon.ico',
                    tag: 'ft-dfrp-network',
                    silent: true
                };
                
                event.waitUntil(
                    self.registration.showNotification('FT-DFRP Network', notificationOptions)
                );
            }
        } catch (e) {
            console.log('[FT-DFRP SW] Invalid push data:', e);
        }
    }
});

// Background network optimization
function optimizeNetworkTopology() {
    const peers = Array.from(networkState.peers.values());
    const optimalConnections = Math.min(5, peers.length);
    
    // Sort peers by connection quality/latency if available
    const sortedPeers = peers.sort((a, b) => {
        const aScore = (a.connectionQuality || 0.5) * (a.reliability || 0.5);
        const bScore = (b.connectionQuality || 0.5) * (b.reliability || 0.5);
        return bScore - aScore;
    });
    
    // Suggest optimal connections to main thread
    notifyMainThread('topology-optimization', {
        suggestedConnections: sortedPeers.slice(0, optimalConnections).map(p => p.nodeId),
        totalPeers: peers.length,
        timestamp: Date.now()
    });
}

// Performance monitoring
let performanceMetrics = {
    messagesSent: 0,
    messagesReceived: 0,
    connectionFailures: 0,
    avgLatency: 0,
    networkEvents: []
};

function updatePerformanceMetrics(metric, value) {
    performanceMetrics[metric] = value;
    
    // Report metrics periodically
    if (performanceMetrics.networkEvents.length > 100) {
        notifyMainThread('performance-report', {
            metrics: performanceMetrics,
            timestamp: Date.now()
        });
        
        // Reset counters
        performanceMetrics.networkEvents = performanceMetrics.networkEvents.slice(-20);
    }
}

console.log('[FT-DFRP SW] Service worker loaded and ready for P2P mesh management');
