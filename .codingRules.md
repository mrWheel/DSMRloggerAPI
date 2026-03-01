# Coding Rules

- AI assistant requirement: Always read this file first and apply all rules at the start of every new chat/session.

- Use the Allman coding style.
- Use lowerCamelCase naming for variables and functions.
- Use 2 spaces for indentation.
- Place comments above variables and functions using the format: `//-- `
- Write all comments in English.
- Keep `README.md` always in English.
- Keep all user-facing and internal code messages in English.
- Keep the `setup()` and `loop()` functions as the last functions in the code.
- Keep `Aandewiel` exactly as written in names; never convert it to lowerCamelCase.
- Treat `PROG_VERSION` as a literal string; never convert it to lowerCamelCase.
- In C/C++ code, prefer `std::string` instead of Arduino `String` where possible.
- In C/C++ code, prefer `Serial.printf()` and `snprintf()` where possible.
- Never remove empty lines or skip code sections.
- For code changes and suggestions, never show only a few lines: always provide complete functions. For new code, clearly indicate where it should be placed by showing the existing lines before and after.
