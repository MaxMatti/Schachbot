#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import subprocess
from PIL import Image, ImageDraw
from sys import argv
from tempfile import mkdtemp

competitors = {
    "getBotMove": ["./getBotMove", "--depth=4"],
    "getPgnMove": ["./getPgnMove", "scoreCacheSorted.txt"],
}

def interpretBoardString(boardString):
    result = []
    for i in boardString:
        if i.isalpha():
            result += [i]
        elif i.isdigit():
            result += [" "] * int(i)
        elif i != "/":
            print("Unknown Symbol: " + i)
    if len(result) < 64:
        result += [""] * (64 - len(result))
    return result

def apply(board, move):
    result = board
    startPos = ord(move[1].lower()) - ord("a") + (ord("8") - ord(move[2])) * 8
    endPos = ord(move[3].lower()) - ord("a") + (ord("8") - ord(move[4])) * 8
    for i in [1, 2, 3, 4, 5, 6, 7, 8]:
        result = result.replace(str(i), " " * i)
    result = result.replace("/", "")
    if result[startPos] == move[0]:
        if move[5] != move[0] and move[5] != " " and move[5] != "\n":
            result = result[:endPos] + move[5] + result[endPos+1:]
        else:
            result = result[:endPos] + result[startPos] + result[endPos+1:]
    else:
        print("Error: \"" + result[startPos] + "\" != \"" + move[0] + "\"")
        exit(0)
    result = result[:startPos] + " " + result[startPos+1:]
    result = '/'.join(result[i:i+8] for i in range(0, len(result), 8))
    for i in [8, 7, 6, 5, 4, 3, 2, 1]:
        result = result.replace(" " * i, str(i))
    if move.startswith("ke8g8"):
        return apply(result, "rh8f8\n")
    elif move.startswith("ke8c8"):
        return apply(result, "ra8d8\n")
    elif move.startswith("Ke1g1"):
        return apply(result, "Rh1f1\n")
    elif move.startswith("Ke1c1"):
        return apply(result, "Ra1d1\n")
    else:
        return result

def display(boardString, filename):
    width = 1366
    height = 768
    command = "/usr/lib/w3m/w3mimgdisplay"

    board = interpretBoardString(boardString)
    pieces = {}
    img = Image.open("board.png")
    for i in range(8):
        for j in range(8):
            char = board[i * 8 + j]
            if char != "" and char != " ":
                if not char in pieces:
                    pieces[char] = Image.open("pieces/" + char + ".png").resize((64, 64), resample=Image.BOX)
                img.paste(pieces[char], (j * 64 + 32, i * 64 + 32), pieces[char].convert("RGBA"))
    img.save(filename)
    width = min(width, img.size[0])
    height = min(height, img.size[1])
    inputstr = ("0;1;0;40;" + str(width) + ";" + str(height) + ";;;;;" + filename + "\n4;\n3;").encode("utf-8")
    subprocess.run([command], input=inputstr, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)

def main():
    for i in competitors:
        for j in competitors:
            knownBoards = []
            currentBoard = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
            filename = "board-" + i + "-" + j + ".png"
            if os.path.isfile(filename):
                continue
            for counter in range(1000):
                knownBoards += [currentBoard]
                currentMove = subprocess.run(competitors[i] + ["--play-white", currentBoard], stdout=subprocess.PIPE).stdout.decode("utf-8")
                print(currentMove[:-1])
                currentBoard = apply(currentBoard, currentMove)
                display(currentBoard, filename)
                if not "K" in currentBoard or not "k" in currentBoard or knownBoards.count(currentBoard) > 10:
                    break
                knownBoards += [currentBoard]
                currentMove = subprocess.run(competitors[j] + ["--play-black", currentBoard], stdout=subprocess.PIPE).stdout.decode("utf-8")
                print(currentMove[:-1])
                currentBoard = apply(currentBoard, currentMove)
                display(currentBoard, filename)
                if not "K" in currentBoard or not "k" in currentBoard or knownBoards.count(currentBoard) > 10:
                    break
            if not "K" in currentBoard:
                print("Tournament: " + i + " vs " + j + ", winner: " + j)
            elif not "k" in currentBoard:
                print("Tournament: " + i + " vs " + j + ", winner: " + i)
            else:
                print("Tournament: " + i + " vs " + j + ", draw.")

if __name__ == '__main__':
    main()
