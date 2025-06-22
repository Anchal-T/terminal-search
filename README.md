# Terminal Search

A powerful, modular terminal-based search application that lets you search the web using the Google, browse results in a modern ncurses UI, and view parsed webpage content—all within your terminal.

---

## Features

- **Terminal UI:** Clean, intuitive ncurses interface with keyboard navigation.
- **Web Search:** Uses Google for fast, relevant results.
- **In-Terminal Browsing:** Select a result to view the parsed, readable webpage content directly in the terminal, with scrolling and navigation.
- **HTML Parsing:** Converts HTML to plain text for optimal terminal display.
- **Secure Credentials:** API keys and search engine IDs are loaded from a `.env` file (never hardcoded).
- **Modular C Code:** Clear separation of HTTP, API, HTML parsing, rendering, and UI logic.
- **Easy Build:** Robust Makefile for building, cleaning, and installing.

---

## Dependencies

- `ncurses`
- `curl`
- `openssl`

**Install on Debian/Ubuntu:**
```bash
sudo apt-get update
sudo apt-get install -y libncurses-dev libcurl4-openssl-dev libssl-dev
```

---

## Quick Start

### 1. Clone the repository
```bash
git clone <repository-url>
cd <repository-directory>
```

### 2. Configure API credentials
- Create a Google Custom Search Engine and get your Search Engine ID (CX).
- Get a Google API Key (enable the "Custom Search API" in Google Cloud Console).
- Copy the example environment file and edit it:
  ```bash
  cp .env.example .env
  # Then edit .env and fill in your API_KEY and CX
  ```

### 3. Build the project
```bash
make
```
The executable will be in the `bin` directory.

### 4. Run the application
```bash
./bin/terminal_search
```

---

## Usage

- **Type your search query** and press `Enter`.
- **Navigate** results with the arrow keys.
- **Press `Enter`** to view a result's webpage content (parsed to plain text).
- **Scroll** in content view with arrow keys or PageUp/PageDown.
- **Press `q`** to go back or quit.

---

## Contributing

Pull requests are welcome! For major changes, please open an issue to discuss your ideas first.

---

## License

[MIT](https://choosealicense.com/licenses/mit/)
