int can_init(can_bitrate_t bitrate) {
  // set rx and tx pins
  CAN_DDR &= ~(1 << CAN_RX);
  CAN_DDR |= (1 << CAN_TX);

  //Reset CAN Controller
  CANGCON |= (1 << SWRES);
  CANGCON = 0x00;


  return 0;
}
