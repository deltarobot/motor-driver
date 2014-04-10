#include "driver.c"
#include "CuTest.h"

#define PWM_CHANNEL 0
#define RANGE 1024
#define TEST_PINS 17
#define DELAY_MS 1

static RPiGPIOPin pins[TEST_PINS] = {
    RPI_V2_GPIO_P1_03,
    RPI_V2_GPIO_P1_05,
    RPI_V2_GPIO_P1_07,
    RPI_V2_GPIO_P1_08,
    RPI_V2_GPIO_P1_10,
    RPI_V2_GPIO_P1_11,
    RPI_V2_GPIO_P1_12,
    RPI_V2_GPIO_P1_13,
    RPI_V2_GPIO_P1_15,
    RPI_V2_GPIO_P1_16,
    RPI_V2_GPIO_P1_18,
    RPI_V2_GPIO_P1_19,
    RPI_V2_GPIO_P1_21,
    RPI_V2_GPIO_P1_22,
    RPI_V2_GPIO_P1_23,
    RPI_V2_GPIO_P1_24,
    RPI_V2_GPIO_P1_26
};

static void startupTest( CuTest *tc );
static void allPinsTest( CuTest *tc );
static void pwmTest( CuTest *tc );
static void i2cTest( CuTest *tc );

static void startupTest( CuTest *tc ) {
  //  bcm2835_set_debug( 1 );

    CuAssert( tc, "Did not successfully initialize.", gpioInit() );
}

static void allPinsTest( CuTest *tc ) {
    int i;

    for( i = 0; i < TEST_PINS; ++i ) {
        bcm2835_gpio_fsel( pins[i], BCM2835_GPIO_FSEL_OUTP );
        bcm2835_gpio_write( pins[i], LOW );
    }

    for( i = 0; i < TEST_PINS; ++i ) {
        bcm2835_gpio_write( pins[i], HIGH );
        bcm2835_delay( DELAY_MS );
        bcm2835_gpio_write( pins[i], LOW );
        bcm2835_delay( DELAY_MS );
        bcm2835_gpio_write( pins[i], HIGH );
        bcm2835_delay( DELAY_MS );
        bcm2835_gpio_write( pins[i], LOW );
        bcm2835_delay( DELAY_MS );
    }

    CuAssert( tc, "Drove all pins successfully.", 1 );
}

static void pwmTest( CuTest *tc ) {
    int data = 0;

    bcm2835_gpio_fsel( RPI_GPIO_P1_12, BCM2835_GPIO_FSEL_ALT5 );
    // Clock divider is set to 16.
    // With a divider of 16 and a RANGE of 1024, in MARKSPACE mode,
    // the pulse repetition frequency will be
    // 1.2MHz/1024 = 1171.875Hz, suitable for driving a DC motor with PWM
    bcm2835_pwm_set_clock( BCM2835_PWM_CLOCK_DIVIDER_16 );
    bcm2835_pwm_set_mode( PWM_CHANNEL, 1, 1 );
    bcm2835_pwm_set_range( PWM_CHANNEL, RANGE );

    while( data < RANGE ) {
        bcm2835_pwm_set_data(PWM_CHANNEL, data);
        data += 8;
        bcm2835_delay( DELAY_MS );
    }

    CuAssert( tc, "Successfully drove the PWM pin.", 1 );
}

static void i2cTest( CuTest *tc ) {
    char outputBuffer[2];
    gpoData = 0xAA55;

    outputBuffer[0] = gpoData & 0xFF;
    outputBuffer[1] = gpoData >> 8;

    CuAssert( tc, "Should be equal", ( ( char* )&gpoData )[0] == outputBuffer[0] );
    CuAssert( tc, "Should be equal", ( ( char* )&gpoData )[1] == outputBuffer[1] );

    gpoData = 0xFFFF;
    
    processOutputGpoCommand( 0xAA55, 0x00FF );
    CuAssert( tc, "Should have only affected bits in bitmask.", gpoData == 0xFF55 );
    processSetGpoCommand( 0x000A );
    CuAssert( tc, "Should have set the bits given.", gpoData == 0xFF5F );
    processClearGpoCommand( 0x5005 );
    CuAssert( tc, "Should have cleared the bits given.", gpoData == 0xAF5A );
}

CuSuite* CuGetSuite( void ) {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST( suite, startupTest );
    SUITE_ADD_TEST( suite, allPinsTest );
    SUITE_ADD_TEST( suite, pwmTest );
    SUITE_ADD_TEST( suite, i2cTest );

    return suite;
}
