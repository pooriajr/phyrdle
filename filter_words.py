#!/usr/bin/env python3

# Script to filter out words with duplicate letters from the Wordle list
# and generate a new word_list.h file with the filtered words

# Read the word list
with open('wordle_official_list.txt', 'r') as f:
    words = [line.strip().upper() for line in f if len(line.strip()) == 5]

print(f'Total words in original list: {len(words)}')

# Filter out words with duplicate letters
filtered_words = []
for word in words:
    if len(set(word)) == len(word):  # No duplicate letters
        filtered_words.append(word)

print(f'Words with unique letters only: {len(filtered_words)}')

# Sort the filtered words alphabetically
filtered_words.sort()

# Generate the word_list.h file
with open('filtered_word_list.h', 'w') as f:
    f.write('#ifndef WORD_LIST_H\n')
    f.write('#define WORD_LIST_H\n\n')
    f.write('#include <avr/pgmspace.h>\n\n')
    f.write('// WORD STORAGE OPTIMIZATION\n')
    f.write('// This list contains only 5-letter words with NO duplicate letters\n')
    f.write('// Words are sorted alphabetically for binary search\n\n')
    f.write('// Packed word list - all words concatenated (alphabetically sorted)\n')
    f.write('const char WORD_PACK[] PROGMEM = \n')
    
    # Write words in chunks of 4 per line for readability
    for i in range(0, len(filtered_words), 4):
        chunk = filtered_words[i:i+4]
        line = '  "' + '"\n  "'.join(chunk) + '"\n'
        f.write(line)
    
    f.write(';\n\n')
    f.write(f'// Number of words in the dictionary\n')
    f.write(f'const int WORD_COUNT = {len(filtered_words)};\n\n')
    f.write('#endif // WORD_LIST_H\n')

print('Generated filtered_word_list.h with words containing no duplicate letters')

# Print first 10 and last 10 words for verification
print('\nFirst 10 words:')
for word in filtered_words[:10]:
    print(word)
    
print('\nLast 10 words:')
for word in filtered_words[-10:]:
    print(word) 