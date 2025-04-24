# Paultry

A secure password manager with a REST API server built with ElysiaJS and Bun, and a C-based CLI client for easy access to your passwords.

## Features

- **Secure Storage**: All passwords are encrypted before being stored in the SQLite database
- **REST API**: Access your passwords programmatically via a RESTful API
- **CLI Client**: Manage your passwords from the command line
- **API Key Authentication**: Secure access with API key authentication
- **Cross-Platform**: Works on Linux, macOS, and Windows

## Server Setup

### Prerequisites

- [Bun](https://bun.sh/) runtime

### Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/q4ow/paultry.git
   cd paultry
   ```

2. Install dependencies:
   ```bash
   bun install
   ```

3. Create a `.env` file in the root directory:
   ```
   PORT=3000
   API_KEY=your_api_key
   DB_PATH=./data/vault.db
   ENCRYPTION_KEY=secure_encryption_key
   ```

4. Start the server:
   ```bash
   bun start
   ```

The server will be running at http://localhost:3000 (or the port you specified).

## API Documentation

Once the server is running, you can access the Swagger documentation at http://localhost:3000/swagger.

### Endpoints

- `GET /passwords` - List all passwords
- `GET /passwords/:id` - Get a specific password
- `POST /passwords` - Add a new password
- `PUT /passwords/:id` - Update a password
- `DELETE /passwords/:id` - Delete a password

All endpoints require the `x-api-key` header with your API key.

## CLI Client

### Prerequisites

- GCC or Clang compiler
- libcurl and libjson-c development libraries

### Installation

1. Navigate to the CLI directory:
   ```bash
   cd cli
   ```

2. Build the CLI:
   ```bash
   make
   ```

3. Configure the CLI:
   ```bash
   ./vault configure
   ```
   You'll be prompted to enter the server URL and API key.

### Usage

```bash
# List all passwords
./vault list

# Get a specific password
./vault get <id>

# Add a new password
./vault add

# Update a password
./vault update <id>

# Delete a password
./vault delete <id>

# Show help
./vault help
```

## Security Considerations

- The API key should be kept secret and should be a strong, random string
- The encryption key should also be strong and kept secure
- For production use, consider implementing HTTPS for the server
- Regularly backup your database file

## Development

### Running in Development Mode

```bash
bun dev
```

This will start the server with hot reloading enabled.

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

---

Made with ❤️ by Keiran
