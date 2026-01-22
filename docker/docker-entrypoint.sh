#!/bin/bash
set -e

# Function to gracefully stop all server processes
cleanup() {
    echo "Shutting down Astonia server..."
    pkill -TERM chatserver 2>/dev/null || true
    pkill -TERM server35 2>/dev/null || true
    sleep 2
    pkill -KILL chatserver 2>/dev/null || true
    pkill -KILL server35 2>/dev/null || true
    echo "Shutdown complete."
    exit 0
}

# Trap signals for graceful shutdown
trap cleanup SIGTERM SIGINT SIGQUIT

# Export environment variables for the server and tools
export AS35_DBHOST="${AS35_DBHOST:-db}"
export AS35_DBUSER="${AS35_DBUSER:-root}"
export AS35_DBPASS="${AS35_DBPASS:-astonia}"
export AS35_DBNAME="${AS35_DBNAME:-merc35}"
export AS35_CHATHOST="${AS35_CHATHOST:-localhost}"

# Wait for MySQL to be ready
wait_for_mysql() {
    echo "Waiting for MySQL at ${AS35_DBHOST}..."
    local max_tries=60
    local count=0
    
    while [ $count -lt $max_tries ]; do
        if mysql -h "${AS35_DBHOST}" \
                 -u "${AS35_DBUSER}" -p"${AS35_DBPASS}" \
                 -e "SELECT 1" >/dev/null 2>&1; then
            echo "MySQL is ready!"
            return 0
        fi
        count=$((count + 1))
        echo "MySQL not ready yet... ($count/$max_tries)"
        sleep 2
    done
    
    echo "ERROR: MySQL did not become ready in time"
    return 1
}

# Initialize database if needed
init_database() {
    echo "Checking database..."
    
    # Check if database exists and has tables
    local tables=$(mysql -h "${AS35_DBHOST}" \
                        -u "${AS35_DBUSER}" -p"${AS35_DBPASS}" \
                        -N -e "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema='${AS35_DBNAME}'" 2>/dev/null || echo "0")
    
    if [ "$tables" = "0" ] || [ -z "$tables" ]; then
        echo "Initializing database..."
        mysql -h "${AS35_DBHOST}" \
              -u "${AS35_DBUSER}" -p"${AS35_DBPASS}" \
              < create_tables.sql
        mysql -h "${AS35_DBHOST}" \
              -u "${AS35_DBUSER}" -p"${AS35_DBPASS}" \
              "${AS35_DBNAME}" < merc.sql
        echo "Database initialized."
    else
        echo "Database already initialized ($tables tables found)."
    fi
}

# Start the server processes
start_server() {
    echo "Starting Astonia Community Server..."
    
    # Start chatserver first
    echo "Starting chatserver..."
    ./chatserver &
    sleep 1
    
    # Define areas to start
    AREAS="1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 22 23 24 25 26 28 29 30 33 36 37"
    
    # Start all area servers (no -d flag, run in background with &)
    for area in $AREAS; do
        echo "Starting area $area..."
        ./server35 -a $area -e &
        sleep 0.5
    done
    
    echo "All server processes started!"
    echo "Server is ready for connections."
    
    # Keep the container running and wait for any process to exit
    while true; do
        # Check if critical processes are still running
        if ! pgrep -x chatserver > /dev/null; then
            echo "WARNING: chatserver died, restarting..."
            ./chatserver &
        fi
        
        # Count running server35 processes
        local running
        running=$(pgrep -c server35 2>/dev/null) || running=0
        if [ "$running" -lt 5 ]; then
            echo "WARNING: Only $running server35 processes running. Something may be wrong."
        fi
        
        sleep 10
    done
}

# Create an account (for admin use)
create_account() {
    if [ -z "$2" ] || [ -z "$3" ]; then
        echo "Usage: create_account <email> <password>"
        exit 1
    fi
    ./create_account -e "$2" "$3"
}

# Create a character (for admin use)
create_character() {
    if [ -z "$2" ] || [ -z "$3" ] || [ -z "$4" ]; then
        echo "Usage: create_character <account_id> <name> <class>"
        echo "Classes: MWG (Mage Warrior God), etc."
        exit 1
    fi
    ./create_character -e "$2" "$3" "$4"
}

# Main command handler
case "${1:-start}" in
    start)
        wait_for_mysql
        init_database
        start_server
        ;;
    create_account)
        create_account "$@"
        ;;
    create_character)
        create_character "$@"
        ;;
    init-db)
        wait_for_mysql
        init_database
        echo "Database initialization complete."
        ;;
    bash|sh)
        exec /bin/bash
        ;;
    *)
        echo "Unknown command: $1"
        echo "Available commands: start, create_account, create_character, init-db, bash"
        exit 1
        ;;
esac
