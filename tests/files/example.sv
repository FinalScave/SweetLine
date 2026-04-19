`timescale 1ns/1ps
`define DEMO_VERBOSE 1
`define WIDTH 32

package demo_pkg;
  typedef enum logic [2:0] {
    OP_ADD = 3'd0,
    OP_SUB = 3'd1,
    OP_AND = 3'd2,
    OP_OR  = 3'd3,
    OP_XOR = 3'd4
  } op_t;

  typedef struct packed {
    logic [WIDTH-1:0] lhs;
    logic [WIDTH-1:0] rhs;
    op_t op;
  } txn_t;

  function automatic logic [WIDTH-1:0] sat_add(
    input logic [WIDTH-1:0] a,
    input logic [WIDTH-1:0] b
  );
    logic [WIDTH:0] sum;
    sum = a + b;
    if (sum[WIDTH]) begin
      return {WIDTH{1'b1}};
    end
    return sum[WIDTH-1:0];
  endfunction

  function automatic logic [WIDTH-1:0] apply_op(
    input txn_t t
  );
    unique case (t.op)
      OP_ADD: return sat_add(t.lhs, t.rhs);
      OP_SUB: return t.lhs - t.rhs;
      OP_AND: return t.lhs & t.rhs;
      OP_OR:  return t.lhs | t.rhs;
      default: return t.lhs ^ t.rhs;
    endcase
  endfunction
endpackage

interface bus_if #(parameter int WIDTH = `WIDTH) (input logic clk, input logic rst_n);
  logic [WIDTH-1:0] lhs;
  logic [WIDTH-1:0] rhs;
  logic [WIDTH-1:0] y;
  logic valid;
  (* keep = "true" *) logic ready;

  modport dut (input clk, rst_n, lhs, rhs, valid, output y, ready);
  modport tb  (input clk, rst_n, y, ready, output lhs, rhs, valid);

  task automatic drive(input logic [WIDTH-1:0] a, input logic [WIDTH-1:0] b);
    @(posedge clk);
    lhs <= a;
    rhs <= b;
    valid <= 1'b1;
    @(posedge clk);
    valid <= 1'b0;
  endtask
endinterface

module alu #(parameter int WIDTH = `WIDTH) (
  input  logic clk,
  input  logic rst_n,
  input  logic [WIDTH-1:0] lhs,
  input  logic [WIDTH-1:0] rhs,
  input  demo_pkg::op_t op,
  output logic [WIDTH-1:0] y,
  output logic valid
);
  import demo_pkg::*;
  logic [WIDTH:0] carry;
  logic [WIDTH-1:0] next_y;

  always_comb begin
    next_y = '0;
    valid = 1'b0;
    unique case (op)
      OP_ADD: begin
        {carry, next_y} = lhs + rhs;
        valid = 1'b1;
      end
      OP_SUB: begin
        next_y = lhs - rhs;
        valid = 1'b1;
      end
      OP_AND: begin
        next_y = lhs & rhs;
        valid = 1'b1;
      end
      OP_OR: begin
        next_y = lhs | rhs;
        valid = 1'b1;
      end
      default: begin
        next_y = lhs ^ rhs;
        valid = 1'b1;
      end
    endcase
  end

  always_ff @(posedge clk or negedge rst_n) begin
    if (!rst_n) begin
      y <= '0;
    end else begin
      y <= next_y;
    end
  end

  assert property (@(posedge clk) disable iff (!rst_n)
    valid |-> ##1 (y inside {'0, '1}));

  cover property (@(posedge clk) valid && (op == OP_ADD));
endmodule

class transaction_scoreboard;
  demo_pkg::txn_t last_txn;

  function new();
    last_txn = '{lhs: '0, rhs: '0, op: demo_pkg::OP_ADD};
  endfunction

  function void sample(demo_pkg::txn_t t);
    last_txn = t;
  endfunction
endclass

module top;
  import demo_pkg::*;

  timeunit 1ns;
  timeprecision 1ps;

  logic clk = 0;
  logic rst_n = 0;
  logic [WIDTH-1:0] lhs;
  logic [WIDTH-1:0] rhs;
  logic [WIDTH-1:0] y;
  logic valid;
  op_t op;
  bus_if #(.WIDTH(WIDTH)) bus(clk, rst_n);
  transaction_scoreboard sb = new();

  alu #(.WIDTH(WIDTH)) dut (
    .clk(clk),
    .rst_n(rst_n),
    .lhs(lhs),
    .rhs(rhs),
    .op(op),
    .y(y),
    .valid(valid)
  );

  generate
    if (WIDTH > 16) begin : gen_wide
      localparam int TAPS = 2;
    end else begin : gen_narrow
      localparam int TAPS = 1;
    end
  endgenerate

  always #5 clk = ~clk;

  initial begin
    rst_n = 0;
    lhs = '0;
    rhs = '0;
    op = OP_ADD;
    repeat (2) @(posedge clk);
    rst_n = 1;

    bus.drive(32'd2, 32'd3);
    sb.sample('{lhs: 32'd2, rhs: 32'd3, op: OP_ADD});

    op = OP_ADD;
    lhs = 32'd10;
    rhs = 32'd7;
    @(posedge clk);
    assert (y == 32'd17) else $error("sum mismatch: %0d", y);

`ifdef DEMO_VERBOSE
    $display("alu result=%0d, valid=%0b", y, valid);
`endif

    op = OP_SUB;
    lhs = 32'd10;
    rhs = 32'd4;
    @(posedge clk);
    assert (y == 32'd6) else $fatal(1, "sub mismatch");

    op = OP_AND;
    lhs = 32'hF0F0_F0F0;
    rhs = 32'h0FF0_0FF0;
    @(posedge clk);
    assert (y == 32'h00F0_00F0);

    op = OP_OR;
    lhs = 32'h1000_0001;
    rhs = 32'h0000_1000;
    @(posedge clk);
    assert (y == 32'h1000_1001);

    repeat (4) @(posedge clk);
    $finish;
  end
endmodule
