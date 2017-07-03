
/*
  Morse ident, based on code from:
  http://www.awce.com/cbasic.htm
*/

#include
#include
#define F_CPU 800000UL	      // Sets up the default speed for delay.h
#include
#include

/* dit delay for morse code */
#define DITDELAY 50  /* mS */
#define OUTPORT PORTB
#define OUTPORTDIRECTION DDRB

// the normal output bit
#define OUTBIT 0

// inverted version out bit
#define INVERTED_OUTBIT 1

// tone output bit
#define TONE_OUTBIT 2

void sendchar(int c);
void tone(int length);

int main (void)
{
  // Put beacon message in program memory
  PGM_P *message=PSTR("cq cq cq cq de vk2tpm ");
  OUTPORTDIRECTION = _BV(OUTBIT) | _BV(INVERTED_OUTBIT) | _BV(TONE_OUTBIT);
  
  while (1)
    {
      int c;
      // Send each character in message
      PGM_P ptr=message;
      for (c=pgm_read_byte(ptr); c; c=pgm_read_byte(++ptr))
	{
	  sendchar(c);
	}
      _delay_ms(30000);  // wait 30 seconds
    }
  return 0;
}

/* Send a Morse code element (.- or space) */
void send(char c)
{
  switch (c)
    {
    case '-':
      OUTPORT = _BV(OUTBIT);
      tone(3*DITDELAY);
      OUTPORT = _BV(INVERTED_OUTBIT);
      break;
    case '.':
      OUTPORT = _BV(OUTBIT);
      tone(DITDELAY);
      OUTPORT = _BV(INVERTED_OUTBIT);
      break;
    case ' ':
      _delay_ms(4*DITDELAY);
      break;
    }
  _delay_ms(DITDELAY);  /* inter element space */
}

// play a tone for the given length
void tone(int length)
{
  int i;
  for(i = 0; i < length; i++)
    {
      // leave the other bits in place
      OUTPORT ^= _BV(TONE_OUTBIT);
      _delay_ms(2);
    }
}
// Send a character (made from dots/dashes) from program memory
void sendc(PGM_P s)
{
  int c;
  for (c=pgm_read_byte(s);c;c=pgm_read_byte(++s))
    {
      send(c);
    }
}
// Send an ASCII character
void sendchar(int c)
{
  switch (toupper(c))
    {
    case ' ': sendc(PSTR(" ")); break;
    case 'A': sendc(PSTR(".-")); break;
    case 'B': sendc(PSTR("-...")); break;
    case 'C': sendc(PSTR("-.-.")); break;
    case 'D': sendc(PSTR("-..")); break;
    case 'E': sendc(PSTR(".")); break;
    case 'F': sendc(PSTR("..-.")); break;
    case 'G': sendc(PSTR("--.")); break;
    case 'H': sendc(PSTR("....")); break;
    case 'I': sendc(PSTR("..")); break;
    case 'J': sendc(PSTR(".---")); break;
    case 'K': sendc(PSTR("-.-")); break;
    case 'L': sendc(PSTR(".-..")); break;
    case 'M': sendc(PSTR("--")); break;
    case 'N': sendc(PSTR("-.")); break;
    case 'O': sendc(PSTR("---")); break;
    case 'P': sendc(PSTR(".--.")); break;
    case 'Q': sendc(PSTR("--.-")); break;
    case 'R': sendc(PSTR(".-.")); break;
    case 'S': sendc(PSTR("...")); break;
    case 'T': sendc(PSTR("-")); break;
    case 'U': sendc(PSTR("..-")); break;
    case 'V': sendc(PSTR("...-")); break;
    case 'W': sendc(PSTR(".--")); break;
    case 'X': sendc(PSTR("-..-")); break;
    case 'Y': sendc(PSTR("-.--")); break;
    case 'Z': sendc(PSTR("--..")); break;
    case '1': sendc(PSTR(".----")); break;
    case '2': sendc(PSTR("..---")); break;
    case '3': sendc(PSTR("...--")); break;
    case '4': sendc(PSTR("....-")); break;
    case '5': sendc(PSTR(".....")); break;
    case '6': sendc(PSTR("-....")); break;
    case '7': sendc(PSTR("--...")); break;
    case '8': sendc(PSTR("---..")); break;
    case '9': sendc(PSTR("----.")); break;
    case '0': sendc(PSTR("-----")); break;
    }
  send(' ');
}
