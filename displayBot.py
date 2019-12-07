#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from math import log
from sys import argv
from PIL import Image, ImageDraw, ImageFont
import pyautogui

values = ["values", "strengths", "weaknesses"]
figures = "KQRBNPFkqrbnpf)"

def interpretBot(bot):
    result = {}
    bot = bot.strip()
    if bot.startswith("Bot(") and ")" in bot:
        bot = bot[4:]
        bot = bot[:bot.find(")")]
    bot = bot.split("/")
    if len(bot) != 3:
        print("Skipping" + str(bot) + ".")
        return
    for i in [0,1,2]:
        result[values[i]] = {}
        for j in range(len(figures) - 1):
            figScore = int(bot[i].split(figures[j], 1)[1].split(figures[j+1])[0])
            result[values[i]][figures[j]] = figScore
    return result

# source: https://stackoverflow.com/a/2292690/3729508
def pascalRow(n, memo={}):
    # This returns the nth row of Pascal's Triangle
    if n in memo:
        return memo[n]
    result = [1]
    x, numerator = 1, n
    for denominator in range(1, n//2+1):
        x *= numerator
        x /= denominator
        result.append(x)
        numerator -= 1
    if n&1 == 0:
        # n is even
        result.extend(reversed(result[:-1]))
    else:
        result.extend(reversed(result))
    memo[n] = result
    return result

# source: https://stackoverflow.com/a/2292690/3729508
def makeBezier(xys):
    # xys should be a sequence of 2-tuples (Bezier control points)
    n = len(xys)
    combinations = pascalRow(n-1)
    def bezier(ts):
        # This uses the generalized formula for bezier curves
        # http://en.wikipedia.org/wiki/B%C3%A9zier_curve#Generalization
        result = []
        for t in ts:
            tpowers = (t**i for i in range(n))
            upowers = reversed([(1-t)**i for i in range(n)])
            coefs = [c*a*b for c, a, b in zip(combinations, tpowers, upowers)]
            result.append(
                tuple(sum([coef*p for coef, p in zip(coefs, ps)]) for ps in zip(*xys)))
        return result
    return bezier

def createBezier(coords, curves):
    points = []
    if len(coords) == 4:
        xStart = coords[0]
        yStart = coords[1]
        xEnd = coords[2]
        yEnd = coords[3]
    elif len(coords) == 2 and len(coords[0]) == 2 and len(coords[1]) == 2:
        xStart = coords[0][0]
        yStart = coords[0][1]
        xEnd = coords[1][0]
        yEnd = coords[1][1]
    else:
        return points
    for c in curves:
        xMin = 1.0
        xMax = 0.0
        yMin = 1.0
        yMax = 0.0
        for p in c:
            xMin = min(xMin, p[0])
            xMax = max(xMax, p[0])
            yMin = min(yMin, p[1])
            yMax = max(yMax, p[1])
        resolution = (xMax - xMin) * (xEnd - xStart) + (yMax - yMin) * (yEnd - yStart)
        ts = [t/resolution for t in range(int(resolution + 1))]
        bezier = makeBezier([(p[0] * (xEnd - xStart) + xStart, p[1] * (yEnd - yStart) + yStart) for p in c])
        points.extend(bezier(ts))
    return points

def displayBot(bot, imageSize):
    defaultBot = interpretBot("K10000Q900R500B300N300P100F10000k-10000q-900r-500b-300n-300p-100f10000/K10Q90R50B30N30P10F10k-10q-90r-50b-30n-30p-10f10/K100000Q90R50B30N30P10F10k-100000q-90r-50b-30n-30p-10f10")
    fontPath = "/usr/share/fonts/ubuntu/Ubuntu-R.ttf"
    antiAliasing = 4
    lineWidth = 1
    innerMargins = 1
    outerMargins = 4

    imgSize = (imageSize[0] * antiAliasing, imageSize[1] * antiAliasing)
    img = Image.new("RGB", imgSize, color=(255, 255, 255))
    draw = ImageDraw.Draw(img)
    testFont = ImageFont.truetype(fontPath, 100)
    minValue = 1
    maxValue = 1
    maxNumberWidth = 0
    maxNumberHeight = 0
    maxLabelWidth = 0
    maxLabelHeight = 0
    minLabelSize = sum([len(bot[i]) for i in bot])
    valueCounter = 0
    for i in bot:
        beginOffset = valueCounter
        for j in bot[i]:
            bot[i][j] /= defaultBot[i][j]
            base = 2
            if bot[i][j] > base:
                bot[i][j] = log(bot[i][j], base) * base
            elif bot[i][j] < -base:
                bot[i][j] = log(-bot[i][j], base) * base
            minValue = min(minValue, bot[i][j])
            maxValue = max(maxValue, bot[i][j])
            numberSize = draw.textsize("{:.1f}".format(bot[i][j]), font=testFont)
            maxNumberWidth = max(maxNumberWidth, numberSize[0])
            maxNumberHeight = max(maxNumberHeight, numberSize[1])
            valueCounter += 1
        labelSize = draw.textsize(i, font=testFont)
        maxLabelWidth = max(maxLabelWidth, labelSize[0])
        maxLabelHeight = max(maxLabelHeight, labelSize[1])
        minLabelSize = min(minLabelSize, valueCounter - beginOffset)
        valueCounter += 1
    valueCounter -= 1
    if minValue > 0:
        zeroPoint = int(imgSize[1] * 0.75)
    else:
        minValue -= 0.1
        zeroPoint = int(imgSize[1] * (0.65 * maxValue / (maxValue - minValue) + 0.1))
    numberFont = ImageFont.truetype(fontPath, int(100 * (imgSize[0] * 0.8 / valueCounter - 2 * innerMargins * antiAliasing) / maxNumberWidth))
    labelFont = ImageFont.truetype(fontPath, int(min(100 * (imgSize[0] * 0.8 / valueCounter * minLabelSize - 2 * innerMargins * antiAliasing) / maxLabelWidth, 100 * imgSize[1] * 0.05 / maxLabelHeight)))
    sideLabelInterval = 1
    while sideLabelInterval * 12 < max(maxValue, -minValue):
        sideLabelInterval *= 10

    intervalStart = min(0, minValue * 1.2) - min(0, minValue * 1.2) % sideLabelInterval
    if intervalStart < 0:
        intervalStart += sideLabelInterval
    for i in range(int(min(0, intervalStart)), int(maxValue * 1.2), sideLabelInterval):
        xStart = -1 / valueCounter * imgSize[0] * 0.8 + imgSize[0] * 0.1
        xEnd = (valueCounter + 1) / valueCounter * imgSize[0] * 0.8 + imgSize[0] * 0.1
        yCoord = zeroPoint - i / maxValue * (zeroPoint - imgSize[1] * 0.1)
        xStart -= xStart % antiAliasing
        xEnd -= xEnd % antiAliasing + 1
        yCoord -= yCoord % antiAliasing
        draw.rectangle([xStart, yCoord, xEnd, yCoord + lineWidth * antiAliasing - 1], fill=(0, 0, 0))
        numberSize = draw.textsize("{:.1f}".format(i), font=numberFont)
        altNumberSize = numberFont.getmask("{:.1f}".format(i)).getbbox()
        draw.text((xStart - numberSize[0] - outerMargins * antiAliasing, yCoord - numberSize[1] + altNumberSize[3] / 2), "{:.1f}".format(i), font=numberFont, fill=(0, 0, 0))
        draw.text((xEnd + outerMargins * antiAliasing, yCoord - numberSize[1] + altNumberSize[3] / 2), "{:.1f}".format(i), font=numberFont, fill=(0, 0, 0))

    offset = 0
    for i in bot:
        beginOffset = offset
        for j in bot[i]:
            xStart = offset / valueCounter * imgSize[0] * 0.8 + imgSize[0] * 0.1
            xEnd = (offset + 1) / valueCounter * imgSize[0] * 0.8 + imgSize[0] * 0.1
            if bot[i][j] > 0:
                yStart = zeroPoint - bot[i][j] / maxValue * (zeroPoint - imgSize[1] * 0.1)
                yEnd = zeroPoint
            else:
                yStart = zeroPoint
                yEnd = zeroPoint - bot[i][j] / maxValue * (zeroPoint - imgSize[1] * 0.1)
            xStart -= xStart % antiAliasing
            xEnd -= xEnd % antiAliasing + 1
            yStart -= yStart % antiAliasing
            yEnd -= yEnd % antiAliasing + 1
            if yEnd > yStart:
                draw.rectangle([xStart, yStart, xEnd, yEnd], fill=(255, 0, 0))
            pieceImage = Image.open("pieces/" + j + ".png").resize((int(xEnd - xStart), int(xEnd - xStart)), resample=Image.BOX)
            if yStart < zeroPoint:
                img.paste(pieceImage, (int(xStart), int(yStart - (xEnd - xStart))), pieceImage.convert("RGBA"))
            else:
                img.paste(pieceImage, (int(xStart), int(zeroPoint - (xEnd - xStart))), pieceImage.convert("RGBA"))
            numberSize = draw.textsize("{:.1f}".format(bot[i][j]), font=numberFont)
            if yStart < zeroPoint - numberSize[1]:
                draw.text((int((xStart + xEnd) / 2 - numberSize[0] / 2), int(yStart)), "{:.1f}".format(bot[i][j]), font=numberFont, fill=(0, 0, 0))
            else:
                draw.text((int((xStart + xEnd) / 2 - numberSize[0] / 2), int(zeroPoint)), "{:.1f}".format(bot[i][j]), font=numberFont, fill=(0, 0, 0))
            offset += 1
        curveXStart = beginOffset / valueCounter * imgSize[0] * 0.8 + imgSize[0] * 0.1
        curveXEnd = offset / valueCounter * imgSize[0] * 0.8 + imgSize[0] * 0.1
        curveYStart = imgSize[1] * 0.75
        curveYEnd = imgSize[1] * 0.85
        curveXStart -= curveXStart % antiAliasing
        curveXEnd -= curveXEnd % antiAliasing + 1
        curveYStart -= curveYStart % antiAliasing
        curveYEnd -= curveYEnd % antiAliasing + 1
        curves = [[(0.0, 0.1), (0.0, 0.55), (0.2, 0.55), (0.3, 0.55), (0.5, 0.55), (0.5, 1.0)],
        [(0.5, 1.0), (0.5, 0.55), (0.7, 0.55), (0.8, 0.55), (1.0, 0.55), (1.0, 0.1)],
        [(1.0, 0.0), (1.0, 0.45), (0.8, 0.45), (0.7, 0.45), (0.5, 0.45), (0.5, 0.9)],
        [(0.5, 0.9), (0.5, 0.45), (0.3, 0.45), (0.2, 0.45), (0.0, 0.45), (0.0, 0.0)]]
        draw.polygon(createBezier([curveXStart, curveYStart, curveXEnd, curveYEnd], curves), fill=(0, 0, 0))
        labelSize = draw.textsize(i, font=labelFont)
        draw.text((int((curveXStart + curveXEnd) / 2 - labelSize[0] / 2), imgSize[1] * 0.85), i, font=labelFont, fill=(0, 0, 0))
        offset += 1

    img.resize(imageSize, resample=Image.BOX).show()

def main():
    imageSize = (pyautogui.size().width, pyautogui.size().height)
    if imageSize == (5040, 1050):
        imageSize = (1678, 1023)
    elif imageSize == (1366, 768):
        imageSize = (1362, 724)
    botStrings = argv[1:]
    while True:
        nextBot = input()
        if nextBot == "":
            break
        botStrings.append(nextBot)
    bots = [interpretBot(bot) for bot in botStrings if interpretBot(bot)]
    for bot in bots:
        displayBot(bot, imageSize)

if __name__ == '__main__':
    main()
