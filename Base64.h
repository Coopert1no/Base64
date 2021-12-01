#pragma once
#include <malloc.h>
#include <cassert>
#include <utility>

class Base64
{
public:
  Base64() = default;

  std::pair<char*, size_t> decode(const char* buffer, size_t bufferSize)
  {
    size_t decodedStringSize = static_cast<size_t>((ceilf((float)bufferSize * 0.75f)));

    char* decodedString = (char*)malloc(decodedStringSize + 1);
    assert(decodedString != nullptr);
    decodedString[decodedStringSize] = '\0';

    size_t index = 0;
    for (size_t i = 0; i < bufferSize; i += 4)
    {
      char character = 0;
      // Copy bits from the first encoded character.
      // First character.
      char bufferCharacter = this->asciiToBase64(buffer[i]);
      character = (bufferCharacter & (0b00111111)) << 2;

      if (i + 1 >= bufferSize)
      {
        this->writeDecodedCharacterToBuffer(character, decodedString, index);
        break;
      }

      bufferCharacter = this->asciiToBase64(buffer[i + 1]);
      character = (0b11111100 & character) | ((0b00110000 & bufferCharacter) >> 4);
      this->writeDecodedCharacterToBuffer(character, decodedString, index);

      // Second character.
      character = (bufferCharacter & (0b00001111)) << 4;

      if (i + 2 >= bufferSize)
      {
        this->writeDecodedCharacterToBuffer(character, decodedString, index);
        break;
      }

      bufferCharacter = this->asciiToBase64(buffer[i + 2]);
      // This handles '=' padding.
      if (bufferCharacter == -1)
      {
        character = 0b11110000 & character;
        decodedStringSize -= 2;
        decodedString[decodedStringSize] = '\0';
        break;
      }
      else
      {
        character = (0b11110000 & character) | ((0b00111100 & bufferCharacter) >> 2);
        this->writeDecodedCharacterToBuffer(character, decodedString, index);
      }

      // Third character.
      character = (bufferCharacter & (0b00000011)) << 6;

      if (i + 3 >= bufferSize)
      {
        this->writeDecodedCharacterToBuffer(character, decodedString, index);
        break;
      }

      bufferCharacter = this->asciiToBase64(buffer[i + 3]);
      if (bufferCharacter == -1)
      {
        character = 0b11000000 & character;
        decodedStringSize -= 1;
        decodedString[decodedStringSize] = '\0';
        break;
      }

      character = (0b11000000 & character) | ((0b00111111 & bufferCharacter));
      this->writeDecodedCharacterToBuffer(character, decodedString, index);
    }

    return std::pair<char*, size_t>(decodedString, decodedStringSize);
  }

  char* encode(const char* buffer, size_t bufferSize, bool disablePadding = false)
  {
    size_t encodedStringSize = 4 * (bufferSize / 3);
    size_t padding = encodedStringSize % 4;

    if (!disablePadding)
      encodedStringSize += padding;

    char* encodedString = (char*)malloc(encodedStringSize + 1);
    assert(encodedString);

    encodedString[encodedStringSize] = '\0';

    size_t index = 0;

    for (size_t i = 0; i < bufferSize; i += 3)
    {
      char character;
      char bufferCharacter;

      // First character.
      bufferCharacter = buffer[i];
      character = (bufferCharacter & 0b11111100) >> 2;
      this->writeEncodedCharacterToBuffer(character, encodedString, index);

      // Second character.
      // Copy remaining 2 bits.
      character = (bufferCharacter & 0b00000011) << 4;

      if (i + 1 >= bufferSize)
      {
        character = character & 0b11110000;
        this->writeEncodedCharacterToBuffer(character, encodedString, index);
        break;
      }

      // Read next character.
      bufferCharacter = buffer[i + 1];

      // Copy rest of the bits.
      character = (0b11110000 & character) | ((0b11110000 & bufferCharacter) >> 4);

      this->writeEncodedCharacterToBuffer(character, encodedString, index);

      // Third character.
      // Copy remaining 4 bits.
      character = (0b11000011 & character) | ((0b00001111 & bufferCharacter) << 2);

      if (i + 2 >= bufferSize)
      {
        character = character & 0b11111100;

        this->writeEncodedCharacterToBuffer(character, encodedString, index);
        break;
      }

      bufferCharacter = buffer[i + 2];

      character = (0b11111100 & character) | ((0b11000000 & bufferCharacter) >> 6);
      this->writeEncodedCharacterToBuffer(character, encodedString, index);

      // Fourth character.
      character = (bufferCharacter & 0b00111111);
      this->writeEncodedCharacterToBuffer(character, encodedString, index);
    }

    if (!disablePadding)
    {
      size_t i = 0;
      while (padding != 0)
      {
        encodedString[index + i] = '=';
        --padding;
        ++i;
      }
    }

    return encodedString;
  }

private:
  char asciiToBase64(char ascii)
  {
    if (ascii == '=')
    {
      return -1;
    }

    char index = 0;
    for (char i = 0; i < s_base64CharsSize; ++i)
    {
      if (m_base64Chars[i] == ascii)
        break;

      ++index;
    }

    return index;
  }

  void writeEncodedCharacterToBuffer(char character, char* buffer, size_t& index)
  {
    buffer[index] = m_base64Chars[(size_t)character];
    ++index;
  }

  void writeDecodedCharacterToBuffer(char character, char* buffer, size_t& index)
  {
    buffer[index] = character;
    ++index;
  }

  const char* m_base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  static constexpr size_t s_base64CharsSize = 64;
};
