#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

/* Global variables (from the decompiled code) */
static char input_buffer[256]; /* User input buffer */
static char *key_parts[5];     /* Serial key split into parts */
static char combined_key[64];  /* Combined key for decryption */

/* MD5 Context Structure */
typedef struct
{
  uint32_t state[4];
  uint64_t count;
  unsigned char buffer[64];
} MD5_CTX;

/* Function prototypes */
int validate_serial_key(void);
int check_first_part(void);
int check_second_part(void);
int check_third_part(void);
int check_fourth_part(void);
int check_fifth_part(void);
void extract_game(void);
uint32_t rotate_left(uint32_t value, uint32_t amount);
void md5_init(MD5_CTX *ctx);
void md5_update(MD5_CTX *ctx, const unsigned char *input, size_t len);
void md5_final(MD5_CTX *ctx, unsigned char digest[16]);
void md5_block(MD5_CTX *ctx, const unsigned char block[64]);
int hex_digit_to_int(char c);
void hex_string_to_bytes(const char *hex, unsigned char *bytes);
int get_next_digit(int *number);

int main(void)
{
  printf("Welcome to Pengu Game!\n");
  printf("Please enter a valid serial key to continue: ");

  /* Read the serial key from user */
  if (scanf("%255s", input_buffer) != 1)
  {
    printf("Error reading input!\n");
    return 1;
  }

  /* Validate the serial key */
  if (validate_serial_key() == 0)
  {
    printf("Invalid serial key!\n");
  }
  else
  {
    printf("If the serial key is legit, you will get your own copy !!\n");
    extract_game();
  }

  return 0;
}

/*
 * Main serial key validation function
 * The key must be 24 characters long and split into 5 parts by dashes
 * Format: XXXX-XXXX-XXXX-XXXX-XXXX
 */
int validate_serial_key(void)
{
  int len;
  int i;

  /* Check if the input is exactly 24 characters (including dashes) */
  len = strlen(input_buffer);
  if (len != 24)
  {
    printf("ERROR: Serial key must be exactly 24 characters long (got %d)\n", len);
    printf("Expected format: XXXX-XXXX-XXXX-XXXX-XXXX\n");
    return 0;
  }

  /* Split the key by dashes */
  key_parts[0] = strtok(input_buffer, "-");
  if (key_parts[0] == NULL)
  {
    printf("ERROR: Invalid format - missing first part\n");
    return 0;
  }

  for (i = 1; i < 5; i++)
  {
    key_parts[i] = strtok(NULL, "-");
    if (key_parts[i] == NULL)
    {
      printf("ERROR: Invalid format - missing part %d (expected 5 parts separated by dashes)\n", i + 1);
      return 0;
    }
  }

  printf("Serial key format: OK\n");
  printf("Validating parts: %s-%s-%s-%s-%s\n",
         key_parts[0], key_parts[1], key_parts[2], key_parts[3], key_parts[4]);

  /* Validate each part of the serial key */
  printf("\nChecking Part 1 (MD5 hash validation)...\n");
  if (check_first_part() == 0)
  {
    printf("FAILED: Part 1 validation failed\n");
    return 0;
  }
  printf("PASSED: Part 1 validation successful\n");

  printf("\nChecking Part 2 (current year validation)...\n");
  if (check_second_part() == 0)
  {
    printf("FAILED: Part 2 validation failed\n");
    return 0;
  }
  printf("PASSED: Part 2 validation successful\n");

  printf("\nChecking Part 3 (XOR pattern validation)...\n");
  if (check_third_part() == 0)
  {
    printf("FAILED: Part 3 validation failed\n");
    return 0;
  }
  printf("PASSED: Part 3 validation successful\n");

  printf("\nChecking Part 4 (digit sum divisible by 7)...\n");
  if (check_fourth_part() == 0)
  {
    printf("FAILED: Part 4 validation failed\n");
    return 0;
  }
  printf("PASSED: Part 4 validation successful\n");

  printf("\nChecking Part 5 (must be '6666')...\n");
  if (check_fifth_part() == 0)
  {
    printf("FAILED: Part 5 validation failed\n");
    return 0;
  }
  printf("PASSED: Part 5 validation successful\n");

  return 1;
}

/*
 * Check first part: Must match a specific MD5 hash
 * The expected hash appears to be hardcoded in the binary
 */
int check_first_part(void)
{
  MD5_CTX ctx;
  unsigned char digest[16];
  unsigned char expected[16];
  int i;

  printf("  - First part: '%s'\n", key_parts[0]);

  /* Check length */
  if (strlen(key_parts[0]) != 4)
  {
    printf("  - ERROR: Part 1 must be exactly 4 characters (got %lu)\n", (unsigned long)strlen(key_parts[0]));
    return 0;
  }

  /* This is the expected MD5 hash (decoded from the hardcoded values) */
  const char expected_hex[] = "df3af285fe2d3226cc36165049ae5425";
  hex_string_to_bytes(expected_hex, expected);

  /* Calculate MD5 of the first part */
  md5_init(&ctx);
  md5_update(&ctx, (unsigned char *)key_parts[0], strlen(key_parts[0]));
  md5_final(&ctx, digest);

  /* Show expected vs actual for debugging */
  printf("  - Expected MD5: ");
  for (i = 0; i < 16; i++)
  {
    printf("%02x", expected[i]);
  }
  printf("\n");
  printf("  - Actual MD5:   ");
  for (i = 0; i < 16; i++)
  {
    printf("%02x", digest[i]);
  }
  printf("\n");

  if (memcmp(digest, expected, 16) != 0)
  {
    printf("  - ERROR: MD5 hash does not match expected value\n");
    return 0;
  }

  return 1;
}

/*
 * Check second part: Must be the current year (e.g., "2025")
 */
int check_second_part(void)
{
  time_t now;
  struct tm *timeinfo;
  int current_year;
  int input_year;

  printf("  - Second part: '%s'\n", key_parts[1]);

  /* Check length */
  if (strlen(key_parts[1]) != 4)
  {
    printf("  - ERROR: Part 2 must be exactly 4 characters (got %lu)\n", (unsigned long)strlen(key_parts[1]));
    return 0;
  }

  time(&now);
  timeinfo = localtime(&now);
  current_year = timeinfo->tm_year + 1900; /* tm_year is years since 1900 */

  input_year = atoi(key_parts[1]);

  printf("  - Current year: %d\n", current_year);
  printf("  - Input year:   %d\n", input_year);

  if (current_year != input_year)
  {
    printf("  - ERROR: Year must match current year (%d)\n", current_year);
    return 0;
  }

  return 1;
}

/*
 * Check third part: XOR with 0x18 must match a specific pattern
 */
int check_third_part(void)
{
  int i;
  const char expected[] = {0x78, 0x78, 0x78, 0x78}; /* Example pattern */

  printf("  - Third part: '%s'\n", key_parts[2]);

  /* Check length */
  if (strlen(key_parts[2]) != 4)
  {
    printf("  - ERROR: Part 3 must be exactly 4 characters (got %lu)\n", (unsigned long)strlen(key_parts[2]));
    return 0;
  }

  printf("  - XOR validation (each char XOR 0x18):\n");
  for (i = 0; i < 4; i++)
  {
    unsigned char actual = key_parts[2][i] ^ 0x18;
    unsigned char expect = expected[i];
    printf("    Position %d: '%c' (0x%02x) XOR 0x18 = 0x%02x, expected 0x%02x",
           i, key_parts[2][i], (unsigned char)key_parts[2][i], actual, expect);

    if (actual != expect)
    {
      printf(" - MISMATCH!\n");
      printf("  - ERROR: XOR pattern validation failed at position %d\n", i);
      return 0;
    }
    printf(" - OK\n");
  }

  return 1;
}

/*
 * Check fourth part: Sum of digits must be divisible by 7
 */
int check_fourth_part(void)
{
  int number, digit_sum = 0;
  int temp_number;

  printf("  - Fourth part: '%s'\n", key_parts[3]);

  /* Check length */
  if (strlen(key_parts[3]) != 4)
  {
    printf("  - ERROR: Part 4 must be exactly 4 characters (got %lu)\n", (unsigned long)strlen(key_parts[3]));
    return 0;
  }

  /* Check if all characters are digits */
  int i;
  for (i = 0; i < 4; i++)
  {
    if (key_parts[3][i] < '0' || key_parts[3][i] > '9')
    {
      printf("  - ERROR: Part 4 must contain only digits (found '%c' at position %d)\n", key_parts[3][i], i);
      return 0;
    }
  }

  number = atoi(key_parts[3]);
  temp_number = number;

  printf("  - Number: %d\n", number);
  printf("  - Calculating digit sum: ");

  /* Sum all digits */
  while (temp_number > 0)
  {
    int digit = get_next_digit(&temp_number);
    digit_sum += digit;
    printf("%d", digit);
    if (temp_number > 0)
      printf(" + ");
  }

  printf(" = %d\n", digit_sum);
  printf("  - %d %% 7 = %d\n", digit_sum, digit_sum % 7);

  if ((digit_sum % 7) != 0)
  {
    printf("  - ERROR: Sum of digits (%d) must be divisible by 7\n", digit_sum);
    return 0;
  }

  return 1;
}

/*
 * Check fifth part: Must be "6666"
 */
int check_fifth_part(void)
{
  printf("  - Fifth part: '%s'\n", key_parts[4]);

  /* Check length */
  if (strlen(key_parts[4]) != 4)
  {
    printf("  - ERROR: Part 5 must be exactly 4 characters (got %lu)\n", (unsigned long)strlen(key_parts[4]));
    return 0;
  }

  /* Check if all characters are digits */
  int i;
  for (i = 0; i < 4; i++)
  {
    if (key_parts[4][i] < '0' || key_parts[4][i] > '9')
    {
      printf("  - ERROR: Part 5 must contain only digits (found '%c' at position %d)\n", key_parts[4][i], i);
      return 0;
    }
  }

  printf("  - Checking if all digits are '6':\n");
  for (i = 0; i < 4; i++)
  {
    printf("    Position %d: '%c'", i, key_parts[4][i]);
    if (key_parts[4][i] != '6')
    {
      printf(" - ERROR: Expected '6'\n");
      printf("  - ERROR: Part 5 must be exactly '6666' (found '%s')\n", key_parts[4]);
      return 0;
    }
    printf(" - OK\n");
  }

  return 1;
}

/*
 * Extract the game file (this appears to decrypt embedded data)
 */
void extract_game(void)
{
  int fd;
  int i;
  char decryption_key[32];

  printf("Extracting game data...\n");

  /* Combine key parts to create decryption key */
  strcpy(combined_key, key_parts[0]);
  strcat(combined_key, key_parts[2]);
  strcat(combined_key, key_parts[4]);

  /* Create the output file */
  fd = open("pengu-game.appimage", O_CREAT | O_WRONLY | O_TRUNC, 0666);
  if (fd == -1)
  {
    perror("open");
    return;
  }

  printf("\n\n\t\tExtracting...\n\n");

  /* Show progress dots */
  printf("\t");
  for (i = 0; i < 26; i++)
  {
    printf(".");
  }
  printf("\r\t");
  fflush(stdout);

  /*
   * This is where the actual game data would be decrypted
   * The original code XORs embedded data with the combined key
   * For demonstration, we'll just show the process
   */
  printf("# Game extraction simulation #\n");
  printf("Your game has been extracted successfully!\n");
  printf("shasum of the game : d77b168b2c963605579203bd3bbcdd6320122eb7\n");

  close(fd);
}

/* Helper function to get next digit from a number */
int get_next_digit(int *number)
{
  int digit = *number % 10;
  *number = *number / 10;
  return digit;
}

/* Rotate left function for MD5 */
uint32_t rotate_left(uint32_t value, uint32_t amount)
{
  return (value << amount) | (value >> (32 - amount));
}

/* MD5 initialization */
void md5_init(MD5_CTX *ctx)
{
  assert(ctx != NULL);

  memset(ctx, 0, sizeof(MD5_CTX));
  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xEFCDAB89;
  ctx->state[2] = 0x98BADCFE;
  ctx->state[3] = 0x10325476;
}

/* Convert hex character to integer */
int hex_digit_to_int(char c)
{
  if (c >= '0' && c <= '9')
  {
    return c - '0';
  }
  else if (c >= 'A' && c <= 'F')
  {
    return c - 'A' + 10;
  }
  else if (c >= 'a' && c <= 'f')
  {
    return c - 'a' + 10;
  }
  else
  {
    assert(0 && "invalid hex character");
    return 0;
  }
}

/* Convert hex string to bytes */
void hex_string_to_bytes(const char *hex, unsigned char *bytes)
{
  printf("hex: %s\n", hex);
  size_t len = strlen(hex);
  int i, byte_index = 0;

  assert(len == 32); /* Expecting 32 hex characters for 16 bytes */

  for (i = 0; i < 32; i += 2)
  {
    int high = hex_digit_to_int(hex[i]);
    int low = hex_digit_to_int(hex[i + 1]);
    bytes[byte_index++] = (high << 4) + low;
  }
  printf("bytes: ");
  for (i = 0; i < byte_index; i++) {
    printf("0x%02x ", bytes[i]);
  }
  printf("\n");
}


/* Simplified MD5 functions - you would need full implementation for actual use */
void md5_update(MD5_CTX *ctx, const unsigned char *input, size_t len)
{
  /* Simplified - real implementation would be more complex */
  /* This is just a placeholder to make the code compile */
  (void)ctx;
  (void)input;
  (void)len;
}

void md5_final(MD5_CTX *ctx, unsigned char digest[16])
{
  /* Simplified - real implementation would be more complex */
  /* This is just a placeholder to make the code compile */
  memset(digest, 0, 16);
  (void)ctx;
}

void md5_block(MD5_CTX *ctx, const unsigned char block[64])
{
  /* This would contain the full MD5 block processing */
  /* Simplified for readability */
  (void)ctx;
  (void)block;
}
