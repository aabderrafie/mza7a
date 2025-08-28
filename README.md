# Pengu Game Serial Key Cracking

## What's this about?

I got this binary called `pengu-game-crack` that asks for a serial key. If you get it right, it extracts some game file. Let's figure out what the key should be!

## How it works

The program is pretty straightforward:

1. Asks you for a serial key
2. Checks if it's valid using 5 different tests
3. If all tests pass, it extracts a game file
4. If not, it just says "Invalid serial key!"

## Serial Key Format

The key needs to be exactly 24 characters long and has 5 parts split by dashes:

```
PART1-PART2-PART3-PART4-PART5
```

## The 5 Tests

### Test 1: MD5 Hash Check

- Takes the first part and calculates its MD5 hash
- Compares it to some encrypted hash stored in the program
- I couldn't figure out what string gives the right hash

### Test 2: Year Check

- Takes the second part and checks if it equals the current year
- **Easy one! Answer: "2025"**

### Test 3: XOR Pattern Check

- Takes the third part and does some XOR magic with 0x18
- Compares result to hardcoded bytes in memory

I used pwndbg to check what bytes it's comparing to:

```
pwndbg> x/4bx 0x5555574bf3c0
0x5555574bf3c0: 0x08    0x81    0x55    0x55
```

So I calculated what the input should be:

- 0x08 ^ 0x18 = 0x10 (not printable!)
- 0x81 ^ 0x18 = 0x99 (not printable!)
- 0x55 ^ 0x18 = 0x4D = 'M'
- 0x55 ^ 0x18 = 0x4D = 'M'

**Problem:** The third part needs non-printable characters, so you can't just type it normally!

### Test 4: Digit Sum Check

- Takes the fourth part and adds up all the digits
- Checks if the sum is divisible by 7
- **Solution: "7000" (7+0+0+0 = 7, which is divisible by 7)**

### Test 5: Final Check

- Takes the fifth part and checks if all 4 characters are the same
- Specifically wants all '6's
- **Solution: "6666"**

## What I tried

I attempted this serial key:

```
INPT-2025-TESE-7000-6666
```

But got this error:

```
pengu-game-crack: tools.c:18: hex_string_to_bytes: Assertion `strlen(hex) == 32' failed.
[1]    38851 IOT instruction  ./pengu-game-crack
```

The program crashed because something's wrong with the first part or how it handles the input.

## What I figured out

**Parts that work:**

- Part 2: "2025" ✓
- Part 4: "7000" ✓
- Part 5: "6666" ✓

**Parts that don't work:**

- Part 1: Need to find what string gives the right MD5 hash
- Part 3: Contains weird non-printable characters that you can't type normally

