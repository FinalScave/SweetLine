`timescale 1ns / 1ps
`default_nettype none

// Reference: https://sutherland-hdl.com/pdfs/verilog_2001_ref_guide.pdf
// Plain Verilog-2001 sample for SweetLine.
// It exercises directives, attributes, modules, tasks, functions, generate
// blocks, specify blocks, named connections, and a small UDP primitive.

`define SAMPLE_WIDTH 8
`define PULSE_CYCLES 3
`define ENABLE_TRACE 1

(* keep = "true", mark_debug = "true" *)
module pulse_stretcher #(
  parameter WIDTH = `SAMPLE_WIDTH,
  parameter PULSE_CYCLES = `PULSE_CYCLES
) (
  input wire clk,
  input wire rst_n,
  input wire enable,
  input wire [WIDTH-1:0] din,
  output reg [WIDTH-1:0] pulse,
  output reg done
);
  localparam [1:0] STATE_IDLE = 2'b00;
  localparam [1:0] STATE_LOAD = 2'b01;
  localparam [1:0] STATE_HOLD = 2'b10;

  wire [WIDTH-1:0] masked_data;
  wire [WIDTH-1:0] tapped_data;
  reg [WIDTH-1:0] shadow;
  reg [1:0] state;
  reg [1:0] next_state;
  integer bit_index;
  time last_edge;
  real duty_scale;
  tri [WIDTH-1:0] debug_bus;
  wand ack_bus;
  wire \escaped_bus$signal ;

  assign masked_data = enable ? din : {WIDTH{1'b0}};
  assign tapped_data = pulse ^ masked_data;
  assign debug_bus = pulse;
  assign ack_bus = enable;
  assign \escaped_bus$signal = pulse[0];

  function [WIDTH-1:0] parity_reduce;
    input [WIDTH-1:0] value;
    integer i;
    begin
      parity_reduce = {WIDTH{1'b0}};
      for (i = 0; i < WIDTH; i = i + 1) begin
        parity_reduce[i] = value[i] ^ shadow[i];
      end
    end
  endfunction

  task clear_outputs;
    output [WIDTH-1:0] target;
    output target_done;
    begin
      target = {WIDTH{1'b0}};
      target_done = 1'b0;
    end
  endtask

  genvar g;
  generate
    for (g = 0; g < WIDTH; g = g + 1) begin : gen_taps
      wire tap_enabled;
      assign tap_enabled = masked_data[g] ^ pulse[g];
    end
  endgenerate

  always @* begin
    next_state = state;
    shadow = masked_data;

    casez (state)
      STATE_IDLE: begin
        if (enable) begin
          next_state = STATE_LOAD;
        end else begin
          next_state = STATE_IDLE;
        end
      end
      STATE_LOAD: begin
        next_state = STATE_HOLD;
      end
      STATE_HOLD: begin
        if (pulse == {WIDTH{1'b0}}) begin
          next_state = STATE_IDLE;
        end else begin
          next_state = STATE_HOLD;
        end
      end
      default: begin
        next_state = STATE_IDLE;
      end
    endcase
  end

  always @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      state <= STATE_IDLE;
      pulse <= {WIDTH{1'b0}};
      done <= 1'b0;
      last_edge <= 0;
      duty_scale <= 0.5;
    end else begin
      state <= next_state;
      pulse <= parity_reduce(masked_data);
      done <= (state == STATE_HOLD);
      last_edge <= $time;
    end
  end

  specify
    specparam t_clk_q = 2;
    specparam t_setup = 1;
    (clk => pulse) = (t_clk_q, t_clk_q);
    $setup(posedge clk, posedge rst_n, t_setup);
  endspecify

endmodule

module tb_pulse_stretcher;
  reg clk;
  reg rst_n;
  reg enable;
  reg [7:0] din;
  wire [7:0] pulse;
  wire done;
  wire and_gate_out;
  time last_sample_time;
  real sample_ratio;

  and u_and (and_gate_out, enable, rst_n);

  pulse_stretcher #(
    .WIDTH(8),
    .PULSE_CYCLES(3)
  ) dut (
    .clk(clk),
    .rst_n(rst_n),
    .enable(enable),
    .din(din),
    .pulse(pulse),
    .done(done)
  );

  initial begin
    clk = 1'b0;
    rst_n = 1'b0;
    enable = 1'b0;
    din = 8'h00;
    last_sample_time = 0;
    sample_ratio = 0.25;

    $dumpfile("pulse_stretcher.vcd");
    $dumpvars(0, tb_pulse_stretcher);

    repeat (2) @(posedge clk);
    rst_n = 1'b1;

    fork
      begin : drive_inputs
        din = 8'h3C;
        enable = 1'b1;
        repeat (4) @(posedge clk);
        enable = 1'b0;
        din = 8'hA5;
        repeat (2) @(posedge clk);
        enable = 1'b1;
        last_sample_time = $time;
      end
      begin : monitor_wave
        repeat (10) @(posedge clk);
        $display("time=%0t pulse=%h done=%b and_gate_out=%b", $time, pulse, done, and_gate_out);
      end
    join

    `ifdef ENABLE_TRACE
      $display("trace pulse=%h done=%b ratio=%f", pulse, done, sample_ratio);
    `endif

    $finish;
  end

  always begin
    #5 clk = ~clk;
  end
endmodule

primitive edge_passthrough (y, a);
  output y;
  input a;

  table
    0 : 0;
    1 : 1;
    x : x;
    ? : x;
  endtable
endprimitive
