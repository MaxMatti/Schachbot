#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from PIL import Image, ImageDraw
from sys import argv

def interpretBoardString(boardString):
    result = []
    for i in boardString:
        if i.isalpha():
            result += [i]
        elif i.isdigit():
            result += [" "] * int(i)
        else:
            print("Unknown Symbol: " + i)
    if len(result) < 64:
        result += [" "] * (64 - len(result))
    return result

def main():
    if len(argv) > 2:
        filename = argv[1]
    else:
        filename = ""
    if len(argv) > 1:
        boardString = argv[-1]
    else:
        boardString = "rnbqkbnrpppppppp8888PPPPPPPPRNBQKBNR"
    board = interpretBoardString(boardString)
    pieces = {}
    img = Image.open("board.png")

    for i in range(8):
        for j in range(8):
            char = board[i * 8 + j]
            if char != "" and char != " ":
                if not char in pieces:
                    pieces[char] = Image.open("pieces/" + char + ".png")
                img.paste(pieces[char], (j * 64 + 32, i * 64 + 32), pieces[char].convert("RGBA"))

    if filename != "":
        img.save(filename)
    else:
        img.show()

if __name__ == '__main__':
    main()
