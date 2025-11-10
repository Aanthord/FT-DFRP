#!/bin/bash

# FT-DFRP Cross-Device Networking Setup Script
# Compatible with Linux and macOS
# Author: Michael Doran - Pinnacle Quantum Group

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# ASCII Art Header
echo -e "${GREEN}"
echo "╔═══════════════════════════════════════════════════════════════════════╗"
echo "║                        FT-DFRP SETUP SCRIPT                          ║"
echo "║          Fractal Toroidal Density Field Routing Protocol            ║"
echo "║                   Cross-Device Mesh Networking                       ║"
echo "╚═══════════════════════════════════════════════════════════════════════╝"
echo -e "${NC}"

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_step() {
    echo -e "${PURPLE}[STEP]${NC} $1"
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to get local IP address
get_local_ip() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        LOCAL_IP=$(ifconfig | grep "inet " | grep -v 127.0.0.1 | awk '{print $2}' | head -1)
    else
        # Linux
        LOCAL_IP=$(ip route get 1 | awk '{print $(NF-2); exit}' 2>/dev/null || \
                   hostname -I | awk '{print $1}' 2>/dev/null || \
                   ifconfig | grep "inet " | grep -v 127.0.0.1 | awk '{print $2}' | head -1)
    fi
    echo "$LOCAL_IP"
}

# Function to check Node.js version
check_nodejs() {
    if command_exists node; then
        NODE_VERSION=$(node --version | sed 's/v//')
        MAJOR_VERSION=$(echo $NODE_VERSION | cut -d. -f1)
        if [ "$MAJOR_VERSION" -ge 14 ]; then
            print_success "Node.js $NODE_VERSION found (✓ >= 14.0.0)"
            return 0
        else
            print_error "Node.js $NODE_VERSION found but version 14+ required"
            return 1
        fi
    else
        print_error "Node.js not found"
        return 1
    fi
}

# Function to install Node.js
install_nodejs() {
    print_step "Installing Node.js..."
    
    if [[ "$OSTYPE" == "darwin"* ]]; then
        # macOS
        if command_exists brew; then
            print_status "Installing Node.js via Homebrew..."
            brew install node
        else
            print_error "Homebrew not found. Please install Node.js manually:"
            echo "  1. Visit: https://nodejs.org"
            echo "  2. Download and install Node.js 18+ LTS"
            echo "  3. Run this script again"
            exit 1
        fi
    else
        # Linux
        if command_exists apt-get; then
            # Ubuntu/Debian
            print_status "Installing Node.js via apt..."
            curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
            sudo apt-get install -y nodejs
        elif command_exists yum; then
            # CentOS/RHEL
            print_status "Installing Node.js via yum..."
            curl -fsSL https://rpm.nodesource.com/setup_18.x | sudo bash -
            sudo yum install -y nodejs
        elif command_exists pacman; then
            # Arch Linux
            print_status "Installing Node.js via pacman..."
            sudo pacman -S nodejs npm
        else
            print_error "Package manager not found. Please install Node.js manually:"
            echo "  1. Visit: https://nodejs.org"
            echo "  2. Download and install Node.js 18+ LTS"
            echo "  3. Run this script again"
            exit 1
        fi
    fi
}

# Function to download required files
download_files() {
    print_step "Setting up FT-DFRP files..."
    
    # Create required files if they don't exist
    if [ ! -f "package.json" ]; then
        print_status "Creating package.json..."
        cat > package.json << 'EOF'
{
  "name": "ft-dfrp-signaling-server",
  "version": "1.0.0",
  "description": "Local signaling server for FT-DFRP cross-device networking demo",
  "main": "signaling-server.js",
  "scripts": {
    "start": "node signaling-server.js",
    "dev": "node signaling-server.js 8080"
  },
  "keywords": ["webrtc", "p2p", "mesh", "networking", "demo"],
  "author": "Michael Doran - Pinnacle Quantum Group",
  "license": "AGPL-3.0",
  "dependencies": {
    "ws": "^8.14.0",
    "express": "^4.18.0"
  },
  "engines": {
    "node": ">=14.0.0"
  }
}
EOF
    fi
    
    if [ ! -f "signaling-server.js" ]; then
        print_warning "signaling-server.js not found!"
        print_status "Please download the complete FT-DFRP package including:"
        echo "  - signaling-server.js"
        echo "  - ft-dfrp-crossdevice-demo.html"
        echo "  - package.json"
        echo ""
        echo "You can create a minimal signaling-server.js or download the complete package."
        read -p "Continue with basic setup? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            exit 1
        fi
    fi
}

# Function to install npm dependencies
install_dependencies() {
    print_step "Installing Node.js dependencies..."
    
    if [ -f "package.json" ]; then
        npm install
        print_success "Dependencies installed successfully"
    else
        print_error "package.json not found"
        exit 1
    fi
}

# Function to check firewall and ports
check_ports() {
    print_step "Checking network configuration..."
    
    # Check if port 8080 is available
    if command_exists netstat; then
        if netstat -ln | grep -q ":8080 "; then
            print_warning "Port 8080 is already in use"
            print_status "The script will try to use port 8080. If it fails, kill other processes using this port."
        else
            print_success "Port 8080 is available"
        fi
    fi
    
    # Firewall warnings
    if [[ "$OSTYPE" == "darwin"* ]]; then
        print_status "macOS detected - check System Preferences > Security & Privacy > Firewall if connections fail"
    else
        print_status "Linux detected - ensure firewall allows port 8080 if connections fail"
        if command_exists ufw; then
            print_status "To allow port 8080: sudo ufw allow 8080"
        elif command_exists firewall-cmd; then
            print_status "To allow port 8080: sudo firewall-cmd --permanent --add-port=8080/tcp && sudo firewall-cmd --reload"
        fi
    fi
}

# Function to start the server
start_server() {
    print_step "Starting FT-DFRP signaling server..."
    
    # Get local IP
    LOCAL_IP=$(get_local_ip)
    
    if [ -z "$LOCAL_IP" ]; then
        print_warning "Could not detect local IP address"
        LOCAL_IP="localhost"
    fi
    
    print_success "Network IP detected: $LOCAL_IP"
    
    echo -e "\n${CYAN}╔═══════════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${CYAN}║                        FT-DFRP NETWORK READY                         ║${NC}"
    echo -e "${CYAN}╚═══════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo -e "${GREEN}🌐 CROSS-DEVICE MESH NETWORKING URLs:${NC}"
    echo ""
    echo -e "${YELLOW}Local access:${NC}"
    echo "   http://localhost:8080/ft-dfrp-crossdevice-demo.html"
    echo ""
    echo -e "${YELLOW}Network access (for other devices):${NC}"
    echo "   http://$LOCAL_IP:8080/ft-dfrp-crossdevice-demo.html"
    echo ""
    echo -e "${GREEN}📱 DEVICE SETUP INSTRUCTIONS:${NC}"
    echo ""
    echo -e "${BLUE}PC/Mac/Linux:${NC}   Open the network URL above"
    echo -e "${BLUE}Mobile:${NC}       Connect to same WiFi, then open network URL"
    echo -e "${BLUE}Multiple PCs:${NC}  Each device opens the SAME network URL"
    echo ""
    echo -e "${PURPLE}Expected Results:${NC}"
    echo "  ✅ Signaling: CONNECTED"
    echo "  ✅ Cross-device peer discovery"
    echo "  ✅ WebRTC mesh formation"
    echo "  ✅ Live distributed networking"
    echo ""
    echo -e "${CYAN}Press Ctrl+C to stop the server${NC}"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    # Start the server
    if [ -f "signaling-server.js" ]; then
        node signaling-server.js
    else
        print_error "signaling-server.js not found!"
        print_status "Please download the complete FT-DFRP package"
        exit 1
    fi
}

# Main execution flow
main() {
    print_step "Checking system requirements..."
    
    # Check Node.js
    if ! check_nodejs; then
        print_step "Node.js installation required..."
        read -p "Install Node.js automatically? (y/N): " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            install_nodejs
            if ! check_nodejs; then
                print_error "Node.js installation failed"
                exit 1
            fi
        else
            print_error "Node.js is required. Please install manually and run script again."
            exit 1
        fi
    fi
    
    # Setup files
    download_files
    
    # Install dependencies
    install_dependencies
    
    # Check network
    check_ports
    
    # Start server
    start_server
}

# Handle script interruption
trap 'echo -e "\n${YELLOW}[INFO]${NC} FT-DFRP server stopped"; exit 0' INT

# Run main function
main "$@"
