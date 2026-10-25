// Breathing RGB LED on the iCE40 UP5K (U12).
// RGB0/1/2 are the dedicated open-drain LED pins 39/40/41.
module top (
    output rgb0,
    output rgb1,
    output rgb2
);

    wire clk;
    SB_HFOSC #(
        .CLKHF_DIV("0b10")      // 48 MHz / 4 = 12 MHz
    ) hfosc (
        .CLKHFPU(1'b1),
        .CLKHFEN(1'b1),
        .CLKHF(clk)
    );

    reg [23:0] phase;
    always @(posedge clk)
        phase <= phase + 1'b1;

    wire [7:0] tri0 = phase[23:16];
    wire [7:0] tri1 = phase[21:14];
    wire [7:0] tri2 = phase[22:15];

    wire [7:0] duty0 = (tri0 < 8'd128) ? tri0 : (8'd255 - tri0);
    wire [7:0] duty1 = (tri1 < 8'd128) ? tri1 : (8'd255 - tri1);
    wire [7:0] duty2 = (tri2 < 8'd128) ? tri2 : (8'd255 - tri2);

    reg [7:0] pwm;
    always @(posedge clk)
        pwm <= pwm + 1'b1;

    wire pwm0 = (pwm < duty0);
    wire pwm1 = (pwm < duty1);
    wire pwm2 = (pwm < duty2);

    SB_RGBA_DRV #(
        .CURRENT_MODE("0b0"),
        .RGB0_CURRENT("0b000000"),
        .RGB1_CURRENT("0b000000"),
        .RGB2_CURRENT("0b000000")
    ) rgb_drv (
        .CURREN(1'b1),
        .RGBLEDEN(1'b1),
        .RGB0PWM(pwm0),
        .RGB1PWM(pwm1),
        .RGB2PWM(pwm2),
        .RGB0(rgb0),
        .RGB1(rgb1),
        .RGB2(rgb2)
    );

endmodule
