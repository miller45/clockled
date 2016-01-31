
#ifdef irdbg

//+=============================================================================
// Dump out the decode_results structure.
//
void  dumpInfo (decode_results *results)
{
  // Check if the buffer overflowed
  if (results->overflow) {
    Serial.println("IR code too long. Edit IRremoteInt.h and increase RAWLEN");
    return;
  }

  // Show Encoding standard
  Serial.print("Encoding  : ");
  encoding(results);
  Serial.println("");

  // Show Code & length
  Serial.print("Code      : ");
  ircode(results);
  Serial.print(" (");
  Serial.print(results->bits, DEC);
  Serial.println(" bits)");
}



//+=============================================================================
// Display encoding type
//
void  encoding (decode_results *results)
{
  switch (results->decode_type) {
    default:
    case UNKNOWN:      Serial.print("UNKNOWN");       break ;
    case NEC:          Serial.print("NEC");           break ;
    case SONY:         Serial.print("SONY");          break ;
    case RC5:          Serial.print("RC5");           break ;
    case RC6:          Serial.print("RC6");           break ;
    case DISH:         Serial.print("DISH");          break ;
    case SHARP:        Serial.print("SHARP");         break ;
    case JVC:          Serial.print("JVC");           break ;
    case SANYO:        Serial.print("SANYO");         break ;
    case MITSUBISHI:   Serial.print("MITSUBISHI");    break ;
    case SAMSUNG:      Serial.print("SAMSUNG");       break ;
    case LG:           Serial.print("LG");            break ;
    case WHYNTER:      Serial.print("WHYNTER");       break ;
    case AIWA_RC_T501: Serial.print("AIWA_RC_T501");  break ;
    case PANASONIC:    Serial.print("PANASONIC");     break ;
    case DENON:        Serial.print("Denon");         break ;
  }
}


//+=============================================================================
// Display IR code
//
void  ircode (decode_results *results)
{

  // Print Code
  Serial.print(results->value, HEX);
  Serial.println();
  switch (results->value) {

    case   0xF700FF: Serial.println("MR0C0"); break;
    case   0xF7807F: Serial.println("MR0C1"); break;
    case   0xF740BF: Serial.println("MR0C2"); break;
    case   0xF7C03F: Serial.println("MR0C3"); break;

    case   0xF720DF: Serial.println("MR1C0"); break;
    case   0xF7A05F: Serial.println("MR1C1"); break;
    case   0xF7609F: Serial.println("MR1C2"); break;
    case   0xF7E01F: Serial.println("MR1C3"); break;

    case   0xF710EF: Serial.println("MR2C0"); break;
    case   0xF7906F: Serial.println("MR2C1"); break;
    case   0xF750AF: Serial.println("MR2C2"); break;
    case   0xF7D02F : Serial.println("MR2C3"); break;

    case  0xF730CF: Serial.println("MR3C0"); break;
    case   0xF7B04F: Serial.println("MR3C1"); break;
    case   0xF7708F: Serial.println("MR3C2"); break;
    case   0xF7F00F: Serial.println("MR3C3"); break;

    case   0xF708F7: Serial.println("MR4C0"); break;
    case   0xF78877: Serial.println("MR4C1"); break;
    case   0xF748B7: Serial.println("MR4C2"); break;
    case   0xF7C837: Serial.println("MR4C3"); break;

    case   0xF728D7: Serial.println("MR5C0"); break;
    case   0xF7A857: Serial.println("MR5C1"); break;
    case   0xF76897: Serial.println("MR5C2"); break;
    case   0xF7E817: Serial.println("MR5C3"); break;
  }
}
#endif


#ifdef i2cdbg

void i2c_scan()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknow error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  //delay(5000);           // wait 5 seconds for next scan
}
#endif
