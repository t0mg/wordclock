#include <vector>
#include "Arduino.h"
#include "ClockFace.h"

// A monospace font table structre.
struct FontTable
{
  int characterWidth;
  int characterHeight;
  char firstChar;
  std::vector<std::vector<bool>> data; // ASCII data.
  static std::vector<bool> getCharData(FontTable fontTable, char c)
  {
    int index = c - fontTable.firstChar;
    if (index < 0 || index >= fontTable.data.size())
    {
      return std::vector<bool>();
    }
    return fontTable.data.at(index);
  };
};

// 5x7 font "dot-matrix.ttf" by sylvan.black
// https://fontstruct.com/fontstructions/show/847768/5x7_dot_
const FontTable font5x7 = {
    .characterWidth = 5,
    .characterHeight = 7,
    .firstChar = '!',
    .data = {
        {// !
         false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false, false},
        {// "
         false, true, false, true, false, false, true, false, true, false, false, true, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
        {// #
         false, true, false, true, false, false, true, false, true, false, true, true, true, true, true, false, true, false, true, false, true, true, true, true, true, false, true, false, true, false, false, true, false, true, false},
        {// $
         false, false, true, false, false, false, true, true, true, true, true, false, true, false, false, false, true, true, true, false, false, false, true, false, true, true, true, true, true, false, false, false, true, false, false},
        {// %
         true, true, false, false, false, true, true, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, false, false, true, true, false, false, false, true, true},
        {// &
         false, true, false, false, false, true, false, true, false, false, true, false, true, false, false, false, true, false, false, false, true, false, true, false, true, true, false, false, true, false, false, true, true, false, true},
        {// '
         false, true, true, false, false, false, false, true, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
        {// (
         false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, true, false},
        {// )
         false, true, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false},
        {// *
         false, false, false, false, false, false, false, true, false, false, true, false, true, false, true, false, true, true, true, false, true, false, true, false, true, false, false, true, false, false, false, false, false, false, false},
        {// +
         false, false, false, false, false, false, false, true, false, false, false, false, true, false, false, true, true, true, true, true, false, false, true, false, false, false, false, true, false, false, false, false, false, false, false},
        {// ,
         false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false, false, false, false, true, false, false, false, true, false, false, false},
        {// -
         false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
        {// .
         false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false, false, false, true, true, false, false},
        {// /
         false, false, false, false, false, false, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, false, false, false, false, false, false},
        {// 0
         false, true, true, true, false, true, false, false, false, true, true, false, false, true, true, true, false, true, false, true, true, true, false, false, true, true, false, false, false, true, false, true, true, true, false},
        {// 1
         false, false, true, false, false, false, true, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, true, true, true, false},
        {// 2
         false, true, true, true, false, true, false, false, false, true, false, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, true, true, true, true},
        {// 3
         true, true, true, true, true, false, false, false, true, false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, true, true, false, false, false, true, false, true, true, true, false},
        {// 4
         false, false, false, true, false, false, false, true, true, false, false, true, false, true, false, true, false, false, true, false, true, true, true, true, true, false, false, false, true, false, false, false, false, true, false},
        {// 5
         true, true, true, true, true, true, false, false, false, false, true, true, true, true, false, false, false, false, false, true, false, false, false, false, true, true, false, false, false, true, false, true, true, true, false},
        {// 6
         false, false, true, true, false, false, true, false, false, false, true, false, false, false, false, true, true, true, true, false, true, false, false, false, true, true, false, false, false, true, false, true, true, true, false},
        {// 7
         true, true, true, true, true, false, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false},
        {// 8
         false, true, true, true, false, true, false, false, false, true, true, false, false, false, true, false, true, true, true, false, true, false, false, false, true, true, false, false, false, true, false, true, true, true, false},
        {// 9
         false, true, true, true, false, true, false, false, false, true, true, false, false, false, true, false, true, true, true, true, false, false, false, false, true, false, false, false, true, false, false, true, true, false, false},
        {// :
         false, false, false, false, false, false, true, true, false, false, false, true, true, false, false, false, false, false, false, false, false, true, true, false, false, false, true, true, false, false, false, false, false, false, false},
        {// ;
         false, false, false, false, false, false, true, true, false, false, false, true, true, false, false, false, false, false, false, false, false, true, true, false, false, false, false, true, false, false, false, true, false, false, false},
        {// <
         false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, true, false},
        {// =
         false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false},
        {// >
         false, true, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false},
        {// ?
         false, true, true, true, false, true, false, false, false, true, false, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, false, false, false, false, false, false, true, false, false},
        {// @
         false, true, true, true, false, true, false, false, false, true, false, false, false, false, true, false, true, true, false, true, true, false, true, false, true, true, false, true, false, true, false, true, true, true, false},
        {// A
         false, false, true, false, false, false, true, false, true, false, true, false, false, false, true, true, false, false, false, true, true, true, true, true, true, true, false, false, false, true, true, false, false, false, true},
        {// B
         true, true, true, true, false, true, false, false, false, true, true, false, false, false, true, true, true, true, true, false, true, false, false, false, true, true, false, false, false, true, true, true, true, true, false},
        {// C
         false, true, true, true, false, true, false, false, false, true, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, true, false, true, true, true, false},
        {// D
         true, true, true, false, false, true, false, false, true, false, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, false, true, false, true, true, true, false, false},
        {// E
         true, true, true, true, true, true, false, false, false, false, true, false, false, false, false, true, true, true, true, false, true, false, false, false, false, true, false, false, false, false, true, true, true, true, true},
        {// F
         true, true, true, true, true, true, false, false, false, false, true, false, false, false, false, true, true, true, true, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false},
        {// G
         false, true, true, true, false, true, false, false, false, true, true, false, false, false, false, true, false, true, true, true, true, false, false, false, true, true, false, false, false, true, false, true, true, true, true},
        {// H
         true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, true, true, true, true, true, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true},
        {// I
         false, true, true, true, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, true, true, true, false},
        {// J
         false, false, true, true, true, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, true, false, false, true, false, false, true, true, false, false},
        {// K
         true, false, false, false, true, true, false, false, true, false, true, false, true, false, false, true, true, false, false, false, true, false, true, false, false, true, false, false, true, false, true, false, false, false, true},
        {// L
         true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, true, true, true, true},
        {// M
         true, false, false, false, true, true, true, false, true, true, true, false, true, false, true, true, false, true, false, true, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true},
        {// N
         true, false, false, false, true, true, false, false, false, true, true, true, false, false, true, true, false, true, false, true, true, false, false, true, true, true, false, false, false, true, true, false, false, false, true},
        {// O
         false, true, true, true, false, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, false, true, true, true, false},
        {// P
         true, true, true, true, false, true, false, false, false, true, true, false, false, false, true, true, true, true, true, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false},
        {// Q
         false, true, true, true, false, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, true, false, true, true, false, false, true, false, false, true, true, false, true},
        {// R
         true, true, true, true, false, true, false, false, false, true, true, false, false, false, true, true, true, true, true, false, true, false, true, false, false, true, false, false, true, false, true, false, false, false, true},
        {// S
         false, true, true, true, false, true, false, false, false, true, true, false, false, false, false, false, true, true, true, false, false, false, false, false, true, true, false, false, false, true, false, true, true, true, false},
        {// T
         true, true, true, true, true, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false},
        {// U
         true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, false, true, true, true, false},
        {// V
         true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, false, true, false, true, false, false, false, true, false, false},
        {// W
         true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, true, false, true, true, false, true, false, true, true, false, true, false, true, false, true, false, true, false},
        {// X
         true, false, false, false, true, true, false, false, false, true, false, true, false, true, false, false, false, true, false, false, false, true, false, true, false, true, false, false, false, true, true, false, false, false, true},
        {// Y
         true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, false, true, false, true, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false},
        {// Z
         true, true, true, true, true, false, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, false, true, true, true, true, true},
        {// [
         false, true, true, true, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, true, true, false},
        {// backslash
         false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false, false},
        {// ]
         false, true, true, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, true, true, true, false},
        {// ^
         false, false, true, false, false, false, true, false, true, false, true, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
        {// _
         false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true},
        {// `
         false, true, false, false, false, false, false, true, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
        {// a
         false, false, false, false, false, false, false, false, false, false, false, true, true, true, false, false, false, false, false, true, false, true, true, true, true, true, false, false, false, true, false, true, true, true, true},
        {// b
         true, false, false, false, false, true, false, false, false, false, true, false, true, true, false, true, true, false, false, true, true, false, false, false, true, true, false, false, false, true, true, true, true, true, false},
        {// c
         false, false, false, false, false, false, false, false, false, false, false, true, true, true, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, true, false, true, true, true, false},
        {// d
         false, false, false, false, true, false, false, false, false, true, false, true, true, false, true, true, false, false, true, true, true, false, false, false, true, true, false, false, false, true, false, true, true, true, true},
        {// e
         false, false, false, false, false, false, false, false, false, false, false, true, true, true, false, true, false, false, false, true, true, true, true, true, true, true, false, false, false, false, false, true, true, true, false},
        {// f
         false, false, true, true, false, false, true, false, false, true, false, true, false, false, false, true, true, true, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false},
        {// g
         false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, true, false, true, true, true, true, false, false, false, false, true, false, true, true, true, false},
        {// h
         true, false, false, false, false, true, false, false, false, false, true, false, true, true, false, true, true, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true},
        {// i
         false, false, true, false, false, false, false, false, false, false, false, true, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, true, true, true, false},
        {// j
         false, false, false, true, false, false, false, false, false, false, false, false, true, true, false, false, false, false, true, false, false, false, false, true, false, true, false, false, true, false, false, true, true, false, false},
        {// k
         true, false, false, false, false, true, false, false, false, false, true, false, false, true, false, true, false, true, false, false, true, true, false, false, false, true, false, true, false, false, true, false, false, true, false},
        {// l
         false, true, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, true, true, true, false},
        {// m
         false, false, false, false, false, false, false, false, false, false, true, true, false, true, false, true, false, true, false, true, true, false, true, false, true, true, false, true, false, true, true, false, false, false, true},
        {// n
         false, false, false, false, false, false, false, false, false, false, true, false, true, true, false, true, true, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true},
        {// o
         false, false, false, false, false, false, false, false, false, false, false, true, true, true, false, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, false, true, true, true, false},
        {// p
         false, false, false, false, false, false, false, false, false, false, true, true, true, true, false, true, false, false, false, true, true, true, true, true, false, true, false, false, false, false, true, false, false, false, false},
        {// q
         false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, true, false, true, true, true, true, false, false, false, false, true, false, false, false, false, true},
        {// r
         false, false, false, false, false, false, false, false, false, false, true, false, true, true, false, true, true, false, false, true, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false},
        {// s
         false, false, false, false, false, false, false, false, false, false, false, true, true, true, false, true, false, false, false, false, false, true, true, true, false, false, false, false, false, true, true, true, true, true, false},
        {// t
         false, true, false, false, false, false, true, false, false, false, true, true, true, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, true, false, false, true, true, false},
        {// u
         false, false, false, false, false, false, false, false, false, false, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, true, false, false, true, true, false, true, true, false, true},
        {// v
         false, false, false, false, false, false, false, false, false, false, true, false, false, false, true, true, false, false, false, true, true, false, false, false, true, false, true, false, true, false, false, false, true, false, false},
        {// w
         false, false, false, false, false, false, false, false, false, false, true, false, false, false, true, true, false, false, false, true, true, false, true, false, true, true, false, true, false, true, false, true, false, true, false},
        {// x
         false, false, false, false, false, false, false, false, false, false, true, false, false, false, true, false, true, false, true, false, false, false, true, false, false, false, true, false, true, false, true, false, false, false, true},
        {// y
         false, false, false, false, false, false, false, false, false, false, true, false, false, false, true, true, false, false, false, true, false, true, true, true, true, false, false, false, false, true, false, true, true, true, false},
        {// z
         false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, true, false, false, false, true, false, false, false, true, false, false, false, true, true, true, true, true},
        {// {
         false, false, false, true, false, false, false, true, false, false, false, false, true, false, false, false, true, false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, false, true, false},
        {// |
         false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false, false, false, true, false, false},
        {// }
         false, true, false, false, false, false, false, true, false, false, false, false, true, false, false, false, false, false, true, false, false, false, true, false, false, false, false, true, false, false, false, true, false, false, false},
        {// ~
         false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, false, true, true, false, false, true, false, false, false, false, false, false, false, false, false, false, false}}};