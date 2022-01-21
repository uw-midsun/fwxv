
// Step 1
uint8_t get_signal1(receive_struct* s)
{
    return s->signal1;
}
uint8_t get_signal2(receive_struct* s)
{
    return s->signal1;
}
uint8_t get_signal3(receive_struct* s)
{
    return s->signal1;
}
uint8_t get_signal1(receive_struct* s)
{
    return s->signal1;
}
uint8_t get_signal1(receive_struct* s)
{
    return s->signal1;
}
uint8_t get_signal1(receive_struct* s)
{
    return s->signal1;
}
uint8_t get_signal1(receive_struct* s)
{
    return s->signal1;
}


// First half

// look at firmware_xiv/codegen/out/can_unpack.h
// or just can_unpack.h

void set_signal1(uint8_t data, transmit_struct* s)
{
    s->signal1 = data;
}

