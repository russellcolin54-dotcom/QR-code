/*
 * ============================================================
 *   TERMINAL QR CODE GENERATOR v1.0
 *   Encode & Decode text as binary grid art
 * ============================================================
 *
 * SOURCES CITED:
 * - ASCII/Binary conversion logic adapted from course notes.
 * - Checksum concept: https://en.wikipedia.org/wiki/Checksum
 * - pow() / cmath reference: https://cplusplus.com/reference/cmath/pow/
 * - File I/O (ofstream): zyBooks 9.7
 * - Vector usage: zyBooks ch.7
 * - static_cast usage: https://cplusplus.com/doc/tutorial/typecasting/
 *
 * AI CITATION: Claude (Anthropic) was used to help plan the
 * program structure, unique signature design, and expand the
 * starter code provided by the student into a full solution.
 * All code reviewed and understood by the author.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>
using namespace std;

/*
 * ------------------------------------------------------------
 *  UNIQUE SIGNATURE EXPLAINED:
 *
 *  The QR grid is surrounded by a decorative border made of
 *  '*' and '-' characters (instead of '#' and '.' used inside
 *  the grid for actual data).
 *
 *  TOP BORDER: encodes CREATOR_INITIALS in binary using
 *              '*' = 1  and  '-' = 0
 *              The initials are readable in the top row!
 *
 *  SIDE/BOTTOM BORDERS: alternating * - * - watermark.
 *
 *  The border is AUTOMATICALLY STRIPPED during decoding so it
 *  never corrupts the plaintext message inside.
 * ------------------------------------------------------------
 */

// Change CREATOR_INITIALS to your own initials!
const string CREATOR_INITIALS = "CR";

const char DARK      = '#';   // data bit 1
const char LIGHT     = '.';   // data bit 0
const char SIG_DARK  = '*';   // signature border 1
const char SIG_LIGHT = '-';   // signature border 0

// ============================================================
//  CLASS: QRCode
//  Holds all encoding/decoding state and logic.
// ============================================================
class QRCode {
private:
    string originalText;
    vector<string> binaryVector;  // one 8-bit string per character
    int gridDimensions;
    string paddedBinaryStr;

public:
    // Constructor
    QRCode() : gridDimensions(0) {}

    // Setter
    void setText(const string& text) {
        originalText = text;
    }

    // ---- 1. Reverse a string --------------------------------
    string reverseStr(const string& s) {
        string result = "";
        for (int i = static_cast<int>(s.size()) - 1; i >= 0; i--) {
            result += s[i];
        }
        return result;
    }

    // ---- 2. Convert one char to 8-bit binary string ---------
    string charToBinary(char c) {
        int ascii = static_cast<int>(c);
        string binary = "";
        while (ascii != 0) {
            binary += to_string(ascii % 2);
            ascii /= 2;
        }
        while (static_cast<int>(binary.size()) < 8) {
            binary += "0";
        }
        return reverseStr(binary);
    }

    // ---- 3. Convert 8-bit binary string back to char --------
    char binaryToChar(const string& binary) {
        int ascii = 0;
        string rev = reverseStr(binary);
        for (size_t i = 0; i < rev.size(); i++) {
            if (rev[i] == '1') {
                ascii += static_cast<int>(pow(2, static_cast<int>(i)));
            }
        }
        return static_cast<char>(ascii);
    }

    // ---- 4. Encode full string into binaryVector ------------
    void encodeText() {
        binaryVector.clear();
        for (size_t i = 0; i < originalText.size(); i++) {
            binaryVector.push_back(charToBinary(originalText[i]));
        }
    }

    // ---- 5. Flatten binaryVector to one long binary string --
    string flattenBinary() {
        string flat = "";
        for (size_t i = 0; i < binaryVector.size(); i++) {
            flat += binaryVector[i];
        }
        return flat;
    }

    // ---- 6. Calculate grid size (smallest NxN that fits) ----
    int calcGridSize(int totalBits) {
        int dim = 1;
        while (dim * dim < totalBits) {
            dim++;
        }
        return dim;
    }

    // ---- 7. Checksum: sum of ASCII values mod 256 -----------
    int checksum(const string& text) {
        int sum = 0;
        for (size_t i = 0; i < text.size(); i++) {
            sum += static_cast<int>(text[i]);
        }
        return sum % 256;
    }

    // ---- 8. Build padded binary string (pads with '0') ------
    void buildPaddedBinary() {
        string flat = flattenBinary();
        int totalBits = static_cast<int>(originalText.size()) * 8;
        gridDimensions = calcGridSize(totalBits);
        int gridTotal = gridDimensions * gridDimensions;

        paddedBinaryStr = flat;
        while (static_cast<int>(paddedBinaryStr.size()) < gridTotal) {
            paddedBinaryStr += '0';
        }
    }

    // ---- 9. Build one signature border line -----------------
    string buildSignatureLine(bool isTop, int width) {
        string line = "";
        if (isTop) {
            string sigBits = "";
            for (size_t i = 0; i < CREATOR_INITIALS.size(); i++) {
                sigBits += charToBinary(CREATOR_INITIALS[i]);
            }
            while (static_cast<int>(sigBits.size()) < width) {
                sigBits += (sigBits.size() % 2 == 0) ? "1" : "0";
            }
            for (int i = 0; i < width; i++) {
                line += (sigBits[i] == '1') ? SIG_DARK : SIG_LIGHT;
                line += ' ';
            }
        } else {
            for (int i = 0; i < width; i++) {
                line += (i % 2 == 0) ? SIG_DARK : SIG_LIGHT;
                line += ' ';
            }
        }
        return line;
    }

    // ---- 10. Render the QR grid (returns string, optionally prints) ----
    string renderQR(bool printToConsole) {
        buildPaddedBinary();
        ostringstream out;

        int borderWidth = gridDimensions + 2;

        string topBorder = "  " + buildSignatureLine(true, borderWidth);
        out << topBorder << "\n";

        size_t idx = 0;
        for (int row = 0; row < gridDimensions; row++) {
            out << ((row % 2 == 0) ? SIG_DARK : SIG_LIGHT) << " ";
            for (int col = 0; col < gridDimensions; col++) {
                char cell = (paddedBinaryStr[idx] == '1') ? DARK : LIGHT;
                out << cell << " ";
                idx++;
            }
            out << ((row % 2 == 0) ? SIG_DARK : SIG_LIGHT) << "\n";
        }

        string botBorder = "  " + buildSignatureLine(false, borderWidth);
        out << botBorder << "\n";

        string result = out.str();
        if (printToConsole) {
            cout << result;
        }
        return result;
    }

    // ---- 11. Decode raw binary string back to text ----------
    string decodeBinaryStr(const string& binaryStr) {
        string decoded = "";
        int numChars = static_cast<int>(binaryStr.size()) / 8;
        for (int i = 0; i < numChars; i++) {
            string byte = binaryStr.substr(i * 8, 8);
            char c = binaryToChar(byte);
            if (c == '\0') break;
            decoded += c;
        }
        return decoded;
    }

    // ---- 12. Parse a rendered QR string, strip border, extract binary ----
    string parseQRGrid(const string& qrText) {
        vector<string> lines;
        stringstream ss(qrText);
        string line;
        while (getline(ss, line)) {
            if (!line.empty()) {
                lines.push_back(line);
            }
        }

        if (static_cast<int>(lines.size()) < 3) {
            return "";
        }

        lines.erase(lines.begin());
        lines.erase(lines.end() - 1);

        string binaryStr = "";
        for (size_t r = 0; r < lines.size(); r++) {
            string& row = lines[r];
            vector<char> tokens;
            for (size_t i = 0; i < row.size(); i++) {
                char ch = row[i];
                if (ch == DARK || ch == LIGHT || ch == SIG_DARK || ch == SIG_LIGHT) {
                    tokens.push_back(ch);
                }
            }
            // Skip first (left border) and last (right border) tokens
            for (size_t t = 1; t + 1 < tokens.size(); t++) {
                binaryStr += (tokens[t] == DARK) ? '1' : '0';
            }
        }
        return binaryStr;
    }

    // Getters
    string getOriginalText()  { return originalText; }
    int    getGridDimensions(){ return gridDimensions; }
};

// ============================================================
//  HELPER: Print a section header banner (plain ASCII only)
// ============================================================
void printHeader(const string& title) {
    cout << "\n";
    cout << "+-------------------------------------------------------+\n";
    cout << "|  " << title;
    for (int i = static_cast<int>(title.size()); i < 53; i++) {
        cout << " ";
    }
    cout << "|\n";
    cout << "+-------------------------------------------------------+\n";
}

// ============================================================
//  HELPER: Explain the unique signature to the user
// ============================================================
void printSignatureInfo() {
    printHeader("UNIQUE SIGNATURE EXPLAINED");
    cout << "\n";
    cout << "  This QR code uses a DECORATIVE BORDER as a hidden signature.\n\n";
    cout << "  TOP BORDER  : encodes \"" << CREATOR_INITIALS << "\" in binary using\n";
    cout << "                * (dark) = 1  and  - (light) = 0\n";
    cout << "                The initials are readable from the top row!\n\n";
    cout << "  SIDE/BOTTOM : alternating * - * - pattern (visual watermark)\n\n";
    cout << "  The border is AUTOMATICALLY STRIPPED during decoding,\n";
    cout << "  so it never corrupts your plaintext message.\n";
    cout << "\n";
}

// ============================================================
//  HELPER: Save QR output to a .txt file (zyBooks 9.7)
// ============================================================
void saveToFile(const string& qrOutput, const string& originalText, int chk) {
    string filename;
    cout << "\n  Enter filename (without .txt): ";
    cin.ignore();
    getline(cin, filename);
    filename += ".txt";

    ofstream outFile(filename);
    if (!outFile) {
        cout << "  [ERROR] Could not open file for writing.\n";
        return;
    }

    outFile << "TERMINAL QR CODE - Generated Output\n";
    outFile << "Original text: " << originalText << "\n";
    outFile << "Checksum: " << chk << "\n\n";
    outFile << "--- QR GRID (paste into decode mode) ---\n\n";
    outFile << qrOutput;
    outFile << "\n--- END OF QR ---\n";
    outFile.close();

    cout << "  [OK] Saved to \"" << filename << "\"\n";
}

// ============================================================
//  MODE: ENCODE  (text -> QR grid)
// ============================================================
void runEncodeMode() {
    QRCode qr;
    string inputText;

    printHeader("ENCODE MODE  --  Text to QR Grid");
    cout << "\n  Enter text to encode:\n  > ";
    cin.ignore();
    getline(cin, inputText);

    if (inputText.empty()) {
        cout << "  [ERROR] Empty input.\n";
        return;
    }

    qr.setText(inputText);
    qr.encodeText();
    qr.buildPaddedBinary();

    int chkInput = qr.checksum(inputText);
    cout << "\n  Input checksum  : " << chkInput << "\n";

    printHeader("QR CODE OUTPUT");
    cout << "\n";
    string rendered = qr.renderQR(true);

    // Verify: re-decode the binary and checksum it
    string flatBin = "";
    for (size_t i = 0; i < inputText.size(); i++) {
        int ascii = static_cast<int>(inputText[i]);
        string byte = "";
        for (int b = 7; b >= 0; b--) {
            byte += ((ascii >> b) & 1) ? '1' : '0';
        }
        flatBin += byte;
    }
    string reDecoded = qr.decodeBinaryStr(flatBin);
    int chkEncoded   = qr.checksum(reDecoded);

    cout << "  Encoded checksum: " << chkEncoded << "\n";
    if (chkInput == chkEncoded) {
        cout << "  CHECKSUM MATCH -- data integrity confirmed!\n";
    } else {
        cout << "  CHECKSUM MISMATCH -- something went wrong.\n";
    }

    cout << "\n  Grid size: " << qr.getGridDimensions()
         << " x "            << qr.getGridDimensions() << "\n";

    printSignatureInfo();

    cout << "  Save QR output to a .txt file? (y/n): ";
    char choice;
    cin >> choice;
    if (choice == 'y' || choice == 'Y') {
        saveToFile(rendered, inputText, chkInput);
    }
}

// ============================================================
//  MODE: DECODE  (QR grid -> text)
// ============================================================
void runDecodeMode() {
    QRCode qr;

    printHeader("DECODE MODE  --  QR Grid to Text");
    cout << "\n  Paste your QR grid below.\n";
    cout << "  Type END on its own line when finished:\n\n";
    cin.ignore();

    string qrInput = "";
    string line;
    while (getline(cin, line)) {
        if (line == "END") break;
        qrInput += line + "\n";
    }

    if (qrInput.empty()) {
        cout << "  [ERROR] No input received.\n";
        return;
    }

    string binaryStr = qr.parseQRGrid(qrInput);
    if (binaryStr.empty()) {
        cout << "  [ERROR] Could not parse QR grid.\n";
        return;
    }

    string decoded = qr.decodeBinaryStr(binaryStr);

    printHeader("DECODED MESSAGE");
    cout << "\n  -> \"" << decoded << "\"\n";

    int chk = qr.checksum(decoded);
    cout << "\n  Decoded checksum: " << chk << "\n";
    cout << "  (Compare this to the checksum shown at encode time.)\n";
}

// ============================================================
//  MAIN
// ============================================================
int main() {
    cout << "\n";
    cout << "  ==========================================\n";
    cout << "   TERMINAL QR CODE GENERATOR  |  by " << CREATOR_INITIALS << "\n";
    cout << "  ==========================================\n\n";
    cout << "  Legend:  " << DARK      << " = 1 (dark, data)\n";
    cout << "           " << LIGHT     << " = 0 (light, data)\n";
    cout << "           " << SIG_DARK  << " = signature border (dark)\n";
    cout << "           " << SIG_LIGHT << " = signature border (light)\n\n";
    cout << "  Select mode:\n";
    cout << "    1 -> Encode text to QR grid\n";
    cout << "    2 -> Decode QR grid to text\n";
    cout << "  > ";

    int mode;
    cin >> mode;

    if (mode == 1) {
        runEncodeMode();
    } else if (mode == 2) {
        runDecodeMode();
    } else {
        cout << "  [ERROR] Invalid selection.\n";
        return 1;
    }

    cout << "\n";
    return 0;
}