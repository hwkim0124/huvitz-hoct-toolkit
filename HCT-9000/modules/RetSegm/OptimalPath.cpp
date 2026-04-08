#include "stdafx.h"
#include "OptimalPath.h"
#include "ImageMat.h"
#include "Boundary.h"
#include "SegmImage.h"
#include "SegmLayer.h"
#include "Coarse.h"

#include "CppUtil2.h"

using namespace RetSegm;
using namespace cv;
using namespace CppUtil;

#include <iostream>
#include <iomanip>
using namespace std;


OptimalPath::OptimalPath()
{
}


bool RetSegm::OptimalPath::designLayerContraintsOfEPI(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	int size = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	auto inner = layerInn->getYs();
	auto outer = layerOut->getYs();

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	// Initial move change from the smoothed inner boundary.
	int initY = -1;
	int first = -1;
	int lastY = -1;
	int moves = 0;

	for (int i = 0; i < size; i++) {
		if (inner[i] >= 0) {
			if (initY < 0) {
				initY = inner[i];
			}
			else {
				first = abs(inner[i] - initY);
				break;
			}
		}
	}

	for (int i = 0; i < size; i++) {
		upper[i] = inner[i] - LAYER_EPI_UPPER_SPAN;
		lower[i] = inner[i] + max((outer[i] - inner[i]) / 2, LAYER_EPI_LOWER_SPAN);
		upper[i] = max(upper[i], 0);
		lower[i] = min(lower[i], height - 1);
		moves = (lastY < 0 ? first : abs(inner[i] - lastY));
		moves = (int)(moves * LAYER_EPI_CHANGE_RATIO);
		moves = max(moves, LAYER_EPI_MOVES_MIN);
		moves = min(moves, LAYER_EPI_MOVES_MAX);
		delta[i] = moves;
		lastY = inner[i];
	}
	return true;
}


bool RetSegm::OptimalPath::designLayerContraintsOfEND(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut, 
														std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	int size = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	auto inner = layerInn->getYs();
	auto outer = layerOut->getYs();

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	// Initial move change from the smoothed inner boundary.
	int initY = -1;
	int first = -1;
	int lastY = -1;
	int moves = 0;

	for (int i = 0; i < size; i++) {
		if (outer[i] >= 0) {
			if (initY < 0) {
				initY = outer[i];
			}
			else {
				first = abs(outer[i] - initY);
				break;
			}
		}
	}

	for (int i = 0; i < size; i++) {
		if (outer[i] >= height) {
			upper[i] = height - 1;
			lower[i] = height - 1;
			delta[i] = LAYER_END_MOVES_MAX;
		}
		else {
			upper[i] = outer[i] - LAYER_END_UPPER_SPAN;
			lower[i] = outer[i] + LAYER_END_LOWER_SPAN;;
			upper[i] = max(upper[i], inner[i]);
			lower[i] = min(lower[i], height - 1);
			moves = (lastY < 0 ? first : abs(outer[i] - lastY));
			moves = (int)(moves * LAYER_END_CHANGE_RATIO);
			moves = max(moves, LAYER_END_MOVES_MIN);
			moves = min(moves, LAYER_END_MOVES_MAX);
			delta[i] = moves;
			lastY = outer[i];
		}
	}
	return true;
}


bool RetSegm::OptimalPath::designLayerContraintsOfILM(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerNFL, 
														std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
														bool isDisc, int disc1, int disc2)
{
	int size = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	auto inner = layerILM->getYs();
	auto outer = layerNFL->getYs();

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	bool found = (isDisc && (disc2 - disc1) > 1);
	int upperSpan = (found ? LAYER_ILM_UPPER_SPAN_DISC : LAYER_ILM_UPPER_SPAN);
	int lowerSpan = (found ? LAYER_ILM_LOWER_SPAN_DISC : LAYER_ILM_LOWER_SPAN);
	int movesMin = (found ? LAYER_ILM_MOVES_MIN_DISC : LAYER_ILM_MOVES_MIN);
	int movesMax = (found ? LAYER_ILM_MOVES_MAX_DISC : LAYER_ILM_MOVES_MAX);

	if (found) {
		int moves = 0;
		for (int i = 0; i < size; i++) {
			if (i >= disc1 && i <= disc2) {
				upper[i] = max(inner[i] - 50, 0);
				lower[i] = min(inner[i] + 25, height - 1);

				if (i < (size - 1)) {
					moves = abs(inner[i + 1] - inner[i]) * 2 + 1;
				}
			}
			else {
				upper[i] = max(inner[i] - 50, 0);
				lower[i] = min(inner[i] + 25, height - 1);

				if (i < (size - 1)) {
					moves = abs(inner[i + 1] - inner[i]) + 1;
				}
			}

			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else {
		int moves = 0;
		for (int i = 0; i < size; i++) {
			upper[i] = max(inner[i] - 75, 0);
			lower[i] = min(inner[i] + 25, height - 1);

			if (i < (size - 1)) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	return true;


	if (found) {
		int moves = 0;
		for (int i = 0; i < size; i++) {
			if (i >= disc1 && i <= disc2) {
				upper[i] = max(inner[i] - LAYER_ILM_UPPER_SPAN_DISC, 0);
				lower[i] = min(inner[i] + LAYER_ILM_LOWER_SPAN_DISC, height - 1);
				// lower[i] = min(min(inner[i] + LAYER_ILM_LOWER_SPAN_DISC, outer[i]), height - 1);
				if (i < (size - 1)) {
					moves = abs(inner[i + 1] - inner[i]) * 2 + 1;
				}
			}
			else {
				upper[i] = max(inner[i] - upperSpan, 0);
				// lower[i] = min(min(inner[i] + lowerSpan, outer[i]), height - 1);
				lower[i] = min(inner[i] + LAYER_ILM_LOWER_SPAN_DISC, height - 1);
				if (i < (size - 1)) {
					moves = abs(inner[i + 1] - inner[i]) + 1;
				}
			}

			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else {
		int moves = 0;
		for (int i = 0; i < size; i++) {
			upper[i] = max(inner[i] - upperSpan, 0);
			// lower[i] = min(min(inner[i] + lowerSpan, outer[i]), height - 1);
			lower[i] = min(inner[i] + LAYER_ILM_LOWER_SPAN_DISC, height - 1);

			if (i < (size - 1)) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	return true;

	/*
	// Initial move change from the smoothed inner boundary.
	int initY = -1;
	int first = -1;
	int lastY = -1;

	for (int i = 0; i < size; i++) {
		if (inner[i] >= 0) {
			if (initY < 0) {
				initY = inner[i];
			}
			else {
				first = abs(inner[i] - initY);
				break;
			}
		}
	}

	for (int i = 0; i < size; i++) {
		upper[i] = inner[i] - LAYER_ILM_UPPER_SPAN;
		lower[i] = inner[i] + LAYER_ILM_LOWER_SPAN;

		upper[i] = max(upper[i], 0);
		lower[i] = min(lower[i], height-1);

		moves = (lastY < 0 ? first : abs(inner[i] - lastY));
		moves = (int)(moves * PATH_ILM_CHANGE_RATIO);
		moves = max(moves, PATH_ILM_MOVES_MIN);
		moves = min(moves, PATH_ILM_MOVES_MAX);
		delta[i] = moves;
		lastY = inner[i];

		if (isDisc) {
			upper[i] = inner[i] - LAYER_ILM_UPPER_SPAN;
			// lower[i] = outer[i] + LAYER_ILM_LOWER_SPAN;
			lower[i] = inner[i] + LAYER_ILM_LOWER_SPAN;
			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);

			if (i >= disc1 && i <= disc2) {
				delta[i] = PATH_ILM_MOVES_MAX;
			}
		}
	}
	*/
	return true;
}


bool RetSegm::OptimalPath::designLayerContraintsOfNFL(const SegmImage * imgSrc, SegmLayer* layerILM, SegmLayer * layerNFL, SegmLayer * layerIPL, SegmLayer* layerOut,
														std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
														bool isDisc, int disc1, int disc2)
{
	auto inner = layerILM->getYs();
	auto layer = layerNFL->getYs();
	auto outer = layerIPL->getYs();
	auto ideal = layerOut->getYs();

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	if (width <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(width, -1);
	lower = vector<int>(width, -1);
	delta = vector<int>(width, -1);

	bool found = (isDisc && (disc2 - disc1) > 1);
	const int upperSpan = (found ? LAYER_NFL_UPPER_SPAN_DISC : LAYER_NFL_UPPER_SPAN);
	const int lowerSpan = (found ? LAYER_NFL_LOWER_SPAN_DISC : LAYER_NFL_LOWER_SPAN);
	const int movesMin = (found ? LAYER_NFL_MOVES_MIN_DISC : LAYER_NFL_MOVES_MIN);
	const int movesMax = (found ? LAYER_NFL_MOVES_MAX_DISC : LAYER_NFL_MOVES_MAX);

	if (isDisc) {
		int moves = 0;
		int bound1, bound2, depth1, depth2;
		for (int i = 0; i < width; i++) {
			depth1 = layer[i] - inner[i];
			depth2 = outer[i] - layer[i];

			if (depth1 < 11) {
				bound1 = (int)(depth1 * 0.75f);
				bound2 = max((int)(depth1 * 0.95f), 9);
			}
			else {
				bound1 = (int)(depth1 * 0.45f);
				// bound2 = min((int)(depth1 * 1.0f), (int)(depth2 * 0.95f));
				bound2 = (int)(depth2 * 0.95f);
			}

			upper[i] = max(layer[i] - bound1, inner[i]);
			lower[i] = min(layer[i] + bound2, outer[i]);

			if (i >= disc1 && i <= disc2) {
				if (layer[i] >= ideal[i]) {
					upper[i] = ideal[i] - upperSpan;
					lower[i] = ideal[i] + lowerSpan;
				}
			}

			if (i < (width - 1)) {
				moves = abs(outer[i + 1] - outer[i]);
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else {
		int moves = 0;
		int bound1, bound2, depth1, depth2;
		for (int i = 0; i < width; i++) {
			depth1 = layer[i] - inner[i];
			depth2 = outer[i] - layer[i];

			if (depth1 < 13) {
				bound1 = (int)(depth1 * 0.75f);
				bound2 = (int)(depth1 * 0.25f);
			}
			else {
				bound1 = (int)(depth1 * 0.45f);
				bound2 = min((int)(depth1 * 0.95f), (int)(depth2 * 0.95f));
			}

			upper[i] = max(layer[i] - bound1, inner[i]);
			lower[i] = min(layer[i] + bound2, outer[i]);

			if (i < (width - 1)) {
				moves = abs(outer[i + 1] - outer[i]);
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	return true;

	/*
	if (isDisc) {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * LAYER_NFL_UPPER_SPAN_DISC_BOUND);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * LAYER_NFL_LOWER_SPAN_DISC_BOUND);

			if (i >= disc1 && i <= disc2) {
				if (layer[i] >= ideal[i]) {
					upper[i] = ideal[i] - upperSpan;
					lower[i] = ideal[i] + lowerSpan;
				}
			}

			if (i < (width - 1)) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * LAYER_NFL_UPPER_SPAN_BOUND);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * LAYER_NFL_LOWER_SPAN_BOUND);

			if (i < (width - 1)) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	*/

	/*
	// int ilm_pos, nfl_pos;
	// int start, close;
	int moves = 0;
	for (int i = 0; i < width; i++) {
		if (isDisc) {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * LAYER_NFL_UPPER_SPAN_BOUND);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * LAYER_NFL_LOWER_SPAN_BOUND);

			if (i >= disc1 && i <= disc2) {
				if (layer[i] >= ideal[i]) {
					upper[i] = ideal[i] - upperSpan;
					lower[i] = ideal[i] + lowerSpan;
				}
			}
		}
		else {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * LAYER_NFL_UPPER_SPAN_DISC_BOUND);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * LAYER_NFL_LOWER_SPAN_DISC_BOUND);
		}

		if (i < (width - 1)) {
			moves = abs(inner[i + 1] - inner[i]);
		}

		upper[i] = max(upper[i], 0);
		lower[i] = min(lower[i], height - 1);
		delta[i] = min(max(moves, movesMin), movesMax);
	}
	*/
	return true;

	/*
	for (int c = 0; c < width; c++) 
	{
		ilm_pos = min(inner[c], layer[c]);
		nfl_pos = max(inner[c], layer[c]);

		if (isDisc) {
			// start = max(nfl_pos - 5, ilm_pos);
			// close = min(nfl_pos + 25, outer[c]); // nfl_pos + (nfl_pos - ilm_pos) / 2;
			start = layer[c] - min((int)((nfl_pos - ilm_pos) * 0.75f), 15);
			close = layer[c] + 15; // max((layer[c] - inner[c]) / 2, 5);

			upper[c] = max(start, inner[c]);
			lower[c] = min(close, outer[c]);
			delta[c] = PATH_NFL_MOVES_MIN;

			if (c >= disc1 && c <= disc2) {
				upper[c] = max(nfl_pos - 5, 0);
				lower[c] = min(nfl_pos + 15, height - 1);
				delta[c] = PATH_NFL_MOVES_MAX;
			}
		}
		else {
			start = max(nfl_pos - 7, ilm_pos);
			close = nfl_pos + (nfl_pos - ilm_pos) / 2;

			upper[c] = max(start, 0);
			lower[c] = min(close, height - 1);
			delta[c] = PATH_NFL_MOVES_MIN;
		}
	}
	*/
	return true;
}


bool RetSegm::OptimalPath::designLayerContraintsOfRPE(const SegmImage * imgSrc, SegmLayer * layerIOS, SegmLayer * layerBRM, SegmLayer* layerRPE, SegmLayer * layerOut,
														std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
														bool isDisc, int disc1, int disc2)
{
	auto inner = layerIOS->getYs();
	auto layer = layerRPE->getYs();
	auto outer = layerBRM->getYs();
	auto ideal = layerOut->getYs();
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	if (width <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(width, -1);
	lower = vector<int>(width, -1);
	delta = vector<int>(width, -1);

	bool found = (isDisc && (disc2 - disc1) > 1);
	const int upperSpan = (found ? LAYER_RPE_UPPER_SPAN_DISC : LAYER_RPE_UPPER_SPAN);
	const int lowerSpan = (found ? LAYER_RPE_LOWER_SPAN_DISC : LAYER_RPE_LOWER_SPAN);
	const int movesMin = (found ? LAYER_RPE_MOVES_MIN_DISC : LAYER_RPE_MOVES_MIN);
	const int movesMax = (found ? LAYER_RPE_MOVES_MAX_DISC : LAYER_RPE_MOVES_MAX);

	int moves = 0;

	if (isDisc) {
		for (int i = 0; i < width; i++) {
			// upper[i] = max(layer[i] - (layer[i] - inner[i]) / 3, inner[i]);
			// lower[i] = min(layer[i] + (outer[i] - layer[i]) / 2, outer[i]);
			upper[i] = inner[i] + (outer[i] - inner[i]) / 2;
			lower[i] = outer[i] - (outer[i] - layer[i]) / 3;

			if (i >= disc1 && i <= disc2) {
				if (inner[i] >= ideal[i]) {
					upper[i] = ideal[i] - upperSpan;
					lower[i] = ideal[i] + movesMax; // lowerSpan;
				}
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);

			if (i < (width - 1)) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else {
		for (int i = 0; i < width; i++) {
			// upper[i] = max(layer[i] - (layer[i] - inner[i]) / 3, inner[i]);
			// lower[i] = min(layer[i] + (outer[i] - layer[i]) / 2, outer[i]);
			upper[i] = inner[i] + (outer[i] - inner[i]) / 2;
			lower[i] = outer[i] - (outer[i] - layer[i]) / 3;

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);

			if (i < (width - 1)) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}


	/*
	int moves = 0;
	for (int i = 0; i < width; i++) {
		upper[i] = max(outer[i] - upperSpan, 0);
		lower[i] = min(outer[i] + lowerSpan, height - 1);

		if (i < (width - 1)) {
			moves = abs(inner[i + 1] - inner[i]);
		}
		delta[i] = min(max(moves, movesMin), movesMax);
	}
	*/
	return true;

	/*
	for (int i = 0; i < width; i++) {
		upper[i] = outer[i] - 15; // inner[i] + (int)((outer[i] - inner[i]) * LAYER_IOS_RPE_SPAN_BOUND);
		lower[i] = outer[i] ; // -(int)((outer[i] - inner[i]) * LAYER_RPE_BRM_SPAN_BOUND);
		upper[i] = max(upper[i], 0);
		lower[i] = min(lower[i], height - 1);
		delta[i] = LAYER_RPE_MOVES_MAX;
	}
	*/
	return true;
}


bool RetSegm::OptimalPath::designLayerContraintsOfIPL(const SegmImage * imgSrc, SegmLayer * layerNFL, SegmLayer * layerIPL, SegmLayer * layerOPL, SegmLayer * layerOut,
									std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisc, int disc1, int disc2)
{
	auto inner = layerNFL->getYs();
	auto layer = layerIPL->getYs();
	auto outer = layerOPL->getYs();
	auto ideal = layerOut->getYs();

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	if (width <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(width, -1);
	lower = vector<int>(width, -1);
	delta = vector<int>(width, -1);

	bool found = (isDisc && (disc2 - disc1) > 1);
	const int upperSpan = (found ? LAYER_IPL_UPPER_SPAN_DISC : LAYER_IPL_UPPER_SPAN);
	const int lowerSpan = (found ? LAYER_IPL_LOWER_SPAN_DISC : LAYER_IPL_LOWER_SPAN);
	const int movesMin = (found ? LAYER_IPL_MOVES_MIN_DISC : LAYER_IPL_MOVES_MIN);
	const int movesMax = (found ? LAYER_IPL_MOVES_MAX_DISC : LAYER_IPL_MOVES_MAX);

	if (isDisc) {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * 0.5f);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * 0.5f);

			if (i >= disc1 && i <= disc2) {
				if (layer[i] >= ideal[i]) {
					upper[i] = ideal[i] - upperSpan;
					lower[i] = ideal[i] + lowerSpan;
				}
			}

			if (i < (width - 1)) {
				moves = abs(inner[i + 1] - inner[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
		}
	}
	else {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * 0.5f);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * 0.5f);

			if (i < (width - 1)) {
				moves = abs(inner[i + 1] - inner[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
		}
	}
	return true;


	if (isDisc) {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * LAYER_IPL_UPPER_SPAN_DISC_BOUND);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * LAYER_IPL_LOWER_SPAN_DISC_BOUND);

			if (i >= disc1 && i <= disc2) {
				if (layer[i] >= ideal[i]) {
					upper[i] = ideal[i] - upperSpan;
					lower[i] = ideal[i] + lowerSpan;
				}
			}

			if (i < (width - 1)) {
				moves = abs(inner[i + 1] - inner[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
		}
	}
	else {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * LAYER_IPL_UPPER_SPAN_BOUND);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * LAYER_IPL_LOWER_SPAN_BOUND);

			if (i < (width - 1)) {
				moves = abs(inner[i + 1] - inner[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
		}
	}


	/*
	int moves = 0;
	for (int i = 0; i < width; i++) {
		if (isDisc) {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * 0.05f);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * 0.05f);
		}
		else {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * 0.45f);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * 0.25f);
		}
		if (i < (width - 1)) {
			moves = abs(inner[i + 1] - inner[i]);
		}
		delta[i] = min(max(moves, movesMin), movesMax);
	}
	*/

	return true;
}


bool RetSegm::OptimalPath::designLayerContraintsOfOPL(const SegmImage * imgSrc, SegmLayer * layerIPL, SegmLayer * layerOPL, SegmLayer* layerRPE, SegmLayer* layerOut, 
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisc, int disc1, int disc2)
{
	auto inner = layerIPL->getYs();
	auto layer = layerOPL->getYs();
	auto outer = layerRPE->getYs();
	auto ideal = layerOut->getYs();

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	if (width <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(width, -1);
	lower = vector<int>(width, -1);
	delta = vector<int>(width, -1);

	bool found = (isDisc && (disc2 - disc1) > 1);
	const int upperSpan = (found ? LAYER_OPL_UPPER_SPAN_DISC : LAYER_OPL_UPPER_SPAN);
	const int lowerSpan = (found ? LAYER_OPL_LOWER_SPAN_DISC : LAYER_OPL_LOWER_SPAN);
	const int movesMin = (found ? LAYER_OPL_MOVES_MIN_DISC : LAYER_OPL_MOVES_MIN);
	const int movesMax = (found ? LAYER_OPL_MOVES_MAX_DISC : LAYER_OPL_MOVES_MAX);

	if (isDisc) {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * 0.75f);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * 0.25f);

			if (i >= disc1 && i <= disc2) {
				if (layer[i] >= ideal[i]) {
					upper[i] = ideal[i] - upperSpan;
					lower[i] = ideal[i] + lowerSpan;
				}
			}

			if (i < (width - 1)) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * 0.75f);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * 0.25f);

			if (i < (width - 1)) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	return true;

	if (isDisc) {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * LAYER_OPL_UPPER_SPAN_DISC_BOUND);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * LAYER_OPL_LOWER_SPAN_DISC_BOUND);

			if (i >= disc1 && i <= disc2) {
				if (layer[i] >= ideal[i]) {
					upper[i] = ideal[i] - upperSpan;
					lower[i] = ideal[i] + lowerSpan;
				}
			}

			if (i < (width - 1)) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			upper[i] = layer[i] - (int)((layer[i] - inner[i]) * LAYER_OPL_UPPER_SPAN_BOUND);
			lower[i] = layer[i] + (int)((outer[i] - layer[i]) * LAYER_OPL_LOWER_SPAN_BOUND);

			if (i < (width - 1)) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}

	/*
	int moves = 0;

	for (int i = 0; i < width; i++) {
		upper[i] = layer[i] - (int)((layer[i] - inner[i]) * 0.45f);
		lower[i] = layer[i] + (int)((outer[i] - layer[i]) * 0.05f);

		if (i < (width - 1)) {
			moves = abs(inner[i + 1] - inner[i]);
		}
		delta[i] = min(max(moves, movesMin), movesMax);
	}
	*/

	return true;

}


bool RetSegm::OptimalPath::designLayerContraintsOfIOS(const SegmImage * imgSrc, SegmLayer* layerOPL, SegmLayer * layerIOS, SegmLayer * layerRPE, SegmLayer* layerBRM,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisc, int disc1, int disc2)
{
	auto inner = layerOPL->getYs();
	auto layer = layerIOS->getYs();
	auto outer = layerRPE->getYs();
	auto ideal = layerBRM->getYs();

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	if (width <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(width, -1);
	lower = vector<int>(width, -1);
	delta = vector<int>(width, -1);

	bool found = (isDisc && (disc2 - disc1) > 1);
	const int upperSpan = (found ? LAYER_IOS_UPPER_SPAN_DISC : LAYER_IOS_UPPER_SPAN);
	const int lowerSpan = (found ? LAYER_IOS_LOWER_SPAN_DISC : LAYER_IOS_LOWER_SPAN);
	const int movesMin = (found ? LAYER_IOS_MOVES_MIN_DISC : LAYER_IOS_MOVES_MIN);
	const int movesMax = (found ? LAYER_IOS_MOVES_MAX_DISC : LAYER_IOS_MOVES_MAX);

	if (isDisc) {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			// upper[i] = max(layer[i] - upperSpan, inner[i] + (layer[i] - inner[i]) / 2); // max(inner[i], ideal[i] + (layer[i] - ideal[i]) / 2));
			// lower[i] = min(outer[i] - (outer[i] - layer[i])/3, ideal[i] - 10); // min(outer[i] - lowerSpan, outer[i] + (outer[i] - layer[i]) / 2));
			upper[i] = max(layer[i] - upperSpan, 0);
			lower[i] = min(layer[i] + lowerSpan, outer[i]);
			upper[i] = min(upper[i], lower[i]);

			if (i >= disc1 && i <= disc2) {
				if (layer[i] >= inner[i]) {
					upper[i] = inner[i] - upperSpan;
					lower[i] = inner[i] + movesMax; // lowerSpan;
				}
			}

			if (i < (width - 1)) {
				moves = abs(inner[i + 1] - inner[i]);
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			// upper[i] = max(layer[i] - upperSpan, inner[i] + (layer[i] - inner[i]) / 2); // max(inner[i], ideal[i] + (layer[i] - ideal[i]) / 2));
			// lower[i] = min(outer[i] - (outer[i] - layer[i]) / 3, ideal[i] - 10); // min(outer[i] - lowerSpan, outer[i] + (outer[i] - layer[i]) / 2));
			upper[i] = max(layer[i] - upperSpan, 0);
			lower[i] = min(layer[i] + lowerSpan, outer[i]);
			upper[i] = min(upper[i], lower[i]);

			if (i < (width - 1)) {
				moves = abs(inner[i + 1] - inner[i]);
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}

	/*
	int moves = 0;
	for (int i = 0; i < width; i++) {
		upper[i] = max(inner[i] - upperSpan, 0);
		lower[i] = min(inner[i] + (outer[i] - inner[i])/2, height - 1);

		if (i < (width - 1)) {
			moves = abs(lower[i + 1] - lower[i]);
		}
		delta[i] = min(max(moves, movesMin), movesMax);
	}
	*/
	return true;
}


bool RetSegm::OptimalPath::designLayerContraintsOfBRM(const SegmImage * imgSrc, SegmLayer * layerIOS, SegmLayer * layerRPE, SegmLayer* layerBRM,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisc, int disc1, int disc2)
{
	auto inner = layerIOS->getYs();
	auto outer = layerRPE->getYs();
	auto layer = layerBRM->getYs();

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	if (width <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(width, -1);
	lower = vector<int>(width, -1);
	delta = vector<int>(width, -1);

	bool found = (isDisc && (disc2 - disc1) > 1);
	const int upperSpan = (found ? LAYER_BRM_UPPER_SPAN_DISC : LAYER_BRM_UPPER_SPAN);
	const int lowerSpan = (found ? LAYER_BRM_LOWER_SPAN_DISC : LAYER_BRM_LOWER_SPAN);
	const int movesMin = (found ? LAYER_BRM_MOVES_MIN_DISC : LAYER_BRM_MOVES_MIN);
	const int movesMax = (found ? LAYER_BRM_MOVES_MAX_DISC : LAYER_BRM_MOVES_MAX);

	if (isDisc) {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			// upper[i] = outer[i] + (layer[i] - outer[i])/2; // min(ideal[i], inner[i] + (ideal[i] - inner[i]) / 2); // max(outer[i] - (outer[i] - inner[i]) / 2, inner[i]);
			// lower[i] = min(layer[i] + lowerSpan, height-1);
			upper[i] = outer[i] + (layer[i] - outer[i])/2; // max(outer[i], layer[i] - upperSpan);
			lower[i] = min(layer[i] + lowerSpan, height - 1);

			if (i >= disc1 && i <= disc2) {
				if (layer[i] >= outer[i]) {
					upper[i] = outer[i] - upperSpan;
					lower[i] = outer[i] + movesMax; // lowerSpan * 2;
				}
			}

			if (i < (width - 1)) {
				moves = abs(outer[i + 1] - outer[i]);
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else {
		int moves = 0;
		for (int i = 0; i < width; i++) {
			// upper[i] = outer[i] + (layer[i] - outer[i]) / 2;  // min(ideal[i], inner[i] + (ideal[i] - inner[i]) / 2); //  max(outer[i] - (outer[i] - inner[i]) / 2, inner[i]);
			// lower[i] = min(layer[i] + lowerSpan, height-1);
			upper[i] = outer[i] + (layer[i] - outer[i]) / 2; //max(outer[i], layer[i] - upperSpan);
			lower[i] = min(layer[i] + lowerSpan, height - 1);

			if (i < (width - 1)) {
				moves = abs(outer[i + 1] - outer[i]);
			}

			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}

	/*
	int moves = 0;
	for (int i = 0; i < width; i++) {
		upper[i] = max(inner[i] - upperSpan, 0);
		lower[i] = min(outer[i] + lowerSpan, height - 1);

		if (i < (width - 1)) {
			moves = abs(lower[i + 1] - lower[i]);
		}
		delta[i] = min(max(moves, movesMin), movesMax);
	}
	*/
	return true;
}


bool RetSegm::OptimalPath::createLayerCostMapOfEPI(const SegmImage * imgSrc, SegmImage * imgCost, 
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat;
	srcMat = imgSrc->getCvMatConst();

	int grayMin = (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*LAYER_EPI_STDDEV_TO_GRAY_MAX);

	// Narrow kernel with vertical strip causes ILM hovering over the true surface. 
	Mat kernel = Mat::zeros(LAYER_EPI_KERNEL_ROWS, LAYER_EPI_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = -1;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

	float min_range = (float)grayMin / 255.0f;
	float max_range = (float)grayMax / 255.0f;

	for (int c = 0, r = 0; c < outMat.cols; c++) {
		for (r = upper[c]; r <= lower[c]; r++) {
			float rate = ((float)srcMat.at<uint8_t>(r, c) / (255));
			if (rate >= min_range && rate <= max_range) {
				rate = 0.0f;
			}
			if (outMat.at<float>(r, c) < 0.0f) {
				outMat.at<float>(r, c) *= (1.0f - rate);
			}
		}
	}

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				if (k >= 0 && k < outMat.rows) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createLayerCostMapOfEND(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat;
	srcMat = imgSrc->getCvMatConst();

	int grayMin = 0; // (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*LAYER_END_STDDEV_TO_GRAY_MAX);

	// Narrow kernel with vertical strip causes ILM hovering over the true surface. 
	Mat kernel = Mat::zeros(LAYER_END_KERNEL_ROWS, LAYER_END_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = -1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = +1;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

	float min_range = (float)grayMin / 255.0f;
	float max_range = (float)grayMax / 255.0f;

	for (int c = 0, r = 0; c < outMat.cols; c++) {
		for (r = upper[c]; r <= lower[c]; r++) {
			float rate = ((float)srcMat.at<uint8_t>(r, c) / (255));
			if (rate >= min_range && rate <= max_range) {
				rate = 0.0f;
			}
			if (outMat.at<float>(r, c) < 0.0f) {
				outMat.at<float>(r, c) *= (1.0f - rate);
			}
		}
	}

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				if (k >= 0 && k < outMat.rows) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createLayerCostMapOfILM(const SegmImage * imgSrc, SegmImage * imgCost, 
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isSample)
{
	Mat srcMat, mask;

	imgSrc->getCvMatConst().copyTo(srcMat);

	int grayMin; //  (int)imgSrc->getMean() + imgSrc->getStddev();
	int grayMax; // (int)(imgSrc->getMean() + imgSrc->getStddev()*3.0f); //  PATH_ILM_STDDEV_TO_GRAY_MAX);

	// Narrow kernel with vertical strip causes ILM hovering over the true surface. 
	Mat kernel;
	
	if (isSample) {
		grayMin = (int)imgSrc->getMean();
		grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*PATH_ILM_STDDEV_TO_GRAY_MAX);

		kernel = Mat::zeros(PATH_ILM_KERNEL_ROWS, PATH_ILM_KERNEL_COLS, CV_32F);
	}
	else {
		grayMin = (int)imgSrc->getMean();
		grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * LAYER_ILM_STDDEV_TO_GRAY_MAX);

		mask = srcMat > grayMax;
		srcMat.setTo(grayMax, mask);
		mask = srcMat < grayMin;
		srcMat.setTo(grayMin, mask);

		// kernel = Mat::zeros(LAYER_ILM_KERNEL_ROWS, LAYER_ILM_KERNEL_COLS, CV_32F);
		kernel = Mat::zeros(LAYER_ILM_KERNEL_ROWS, LAYER_ILM_KERNEL_COLS, CV_32F);
	}


	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	/*
	float min_range = (float)grayMin / 255.0f;
	float max_range = (float)grayMax / 255.0f;

	for (int c = 0, r = 0; c < outMat.cols; c++) {
		for (r = upper[c]; r <= lower[c]; r++) {
			float rate = ((float)srcMat.at<uint8_t>(r, c) / (255));
			if (rate >= min_range && rate <= max_range) {
				rate = 0.0f;
			}
			if (outMat.at<float>(r, c) < 0.0f) {
				outMat.at<float>(r, c) *= (1.0f - rate);
			}
		}
	}
	*/

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createLayerCostMapOfNFL(const SegmImage * imgSrc, SegmImage * imgCost, 
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisk)
{
	Mat srcMat;
	// srcMat = imgSrc->getCvMatConst();

	imgSrc->getCvMat().copyTo(srcMat);
	int grayMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * (isDisk ? 0.0f : 1.0f));
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * (isDisk ? 3.0f : 4.0f));

	Mat mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	int k_rows, k_cols;
	if (isDisk) {
		k_rows = LAYER_NFL_KERNEL_ROWS_DISC; //  7; // PATH_NFL_KERNEL_ROWS;
		k_cols = LAYER_NFL_KERNEL_COLS_DISC; // 13; // PATH_NFL_KERNEL_COLS;
	}
	else {
		k_rows = LAYER_NFL_KERNEL_ROWS; //  7; // PATH_NFL_KERNEL_ROWS;
		k_cols = LAYER_NFL_KERNEL_COLS; // 13; // PATH_NFL_KERNEL_COLS;
	}

	Mat kernel = Mat::zeros(k_rows, k_cols, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1.0f;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1.0f;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat, supMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	for (int c = 0, r = 0; c < outMat.cols; c++) {
		int dist = 0;
		for (r = upper[c]; r <= lower[c]; r++, dist++) {
			outMat.at<float>(r, c) += 0.001f * dist;
		}
	}

	if (false) //  isDisk == false)
	{
		k_rows = (int)(PATH_NFL_KERNEL_ROWS * imgSrc->getWidthRatioToSample());
		k_cols = (int)(PATH_NFL_KERNEL_COLS * imgSrc->getHeightRatioToSample());
		k_rows += (k_rows % 2 ? 0 : 1);
		k_cols += (k_cols % 2 ? 0 : 1);

		Mat kernel = Mat::zeros(k_rows, k_cols, CV_32F);
		for (int r = 0; r < kernel.rows / 2; r++) {
			for (int c = 0; c < kernel.cols; c++) {
				kernel.at<float>(r, 0) = -1;
			}
		}
		for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
			for (int c = 0; c < kernel.cols; c++) {
				kernel.at<float>(r, 0) = +1;
			}
		}

		filter2D(srcMat, supMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);
	}

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;
	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createLayerCostMapOfIPL(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisk)
{
	Mat srcMat;
	
	imgSrc->getCvMat().copyTo(srcMat);
	int grayMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * (isDisk ? 0.0f : 0.0f));
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * (isDisk ? 3.0f : 3.0f));

	Mat mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	int k_rows, k_cols;
	if (isDisk) {
		k_rows = LAYER_NFL_KERNEL_ROWS_DISC; //  7; // PATH_NFL_KERNEL_ROWS;
		k_cols = LAYER_NFL_KERNEL_COLS_DISC; // 13; // PATH_NFL_KERNEL_COLS;
	}
	else {
		k_rows = LAYER_NFL_KERNEL_ROWS; //  7; // PATH_NFL_KERNEL_ROWS;
		k_cols = LAYER_NFL_KERNEL_COLS; // 13; // PATH_NFL_KERNEL_COLS;
	}

	Mat kernel = Mat::zeros(k_rows, k_cols, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1.0f;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1.0f;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat, supMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;
	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createLayerCostMapOfOPL(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisk)
{
	Mat srcMat;
	imgSrc->getCvMat().copyTo(srcMat);
	int grayMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * (isDisk ? 0.0f : 0.0f));
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * (isDisk ? 2.0f : 2.0f));

	int k_rows, k_cols;
	if (isDisk) {
		k_rows = LAYER_NFL_KERNEL_ROWS_DISC; //  7; // PATH_NFL_KERNEL_ROWS;
		k_cols = LAYER_NFL_KERNEL_COLS_DISC; // 13; // PATH_NFL_KERNEL_COLS;
	}
	else {
		k_rows = LAYER_NFL_KERNEL_ROWS; //  7; // PATH_NFL_KERNEL_ROWS;
		k_cols = LAYER_NFL_KERNEL_COLS; // 13; // PATH_NFL_KERNEL_COLS;
	}

	Mat kernel = Mat::zeros(k_rows, k_cols, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1.0f;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1.0f;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat, supMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;
	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createLayerCostMapOfRPE(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisk)
{
	Mat srcMat, mask;
	// srcMat = imgSrc->getCvMatConst();
	imgSrc->getCvMat().copyTo(srcMat);

	int grayMin = (int)imgSrc->getMean();
	int grayMax = 255; // (int)(imgSrc->getMean() + imgSrc->getStddev()*PATH_ILM_STDDEV_TO_GRAY_MAX);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	// Narrow kernel with vertical strip causes ILM hovering over the true surface. 
	// Mat kernel = Mat::zeros(PATH_RPE_KERNEL_ROWS, PATH_RPE_KERNEL_COLS, CV_32F);
	Mat kernel = Mat::zeros(LAYER_RPE_KERNEL_ROWS, LAYER_RPE_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	/*
	float min_range = (float)grayMin / 255.0f;
	float max_range = (float)grayMax / 255.0f;

	for (int c = 0, r = 0; c < outMat.cols; c++) {
		for (r = upper[c]; r <= lower[c]; r++) {
			float rate = ((float)srcMat.at<uint8_t>(r, c) / (255));
			if (rate >= min_range && rate <= max_range) {
				rate = 0.0f;
			}
			if (outMat.at<float>(r, c) < 0.0f) {
				outMat.at<float>(r, c) *= (1.0f - rate);
			}
		}
	}
	*/

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createLayerCostMapOfIOS(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisk)
{
	Mat srcMat, mask;
	// srcMat = imgSrc->getCvMatConst();
	imgSrc->getCvMat().copyTo(srcMat);

	int grayMin = (int)(imgSrc->getMean() + imgSrc->getStddev()*1.0f);
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*4.0f);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	// Narrow kernel with vertical strip causes ILM hovering over the true surface. 
	Mat kernel = Mat::zeros(LAYER_IOS_KERNEL_ROWS, LAYER_IOS_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createLayerCostMapOfBRM(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisk)
{
	Mat srcMat, mask;
	// srcMat = imgSrc->getCvMatConst();
	imgSrc->getCvMat().copyTo(srcMat);

	int grayMin = (int)(imgSrc->getMean() + imgSrc->getStddev()*1.0f);
	int grayMax = 255;// (int)(imgSrc->getMean() + imgSrc->getStddev()*6.0f);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	// Narrow kernel with vertical strip causes ILM hovering over the true surface. 
	Mat kernel = Mat::zeros(LAYER_BRM_KERNEL_ROWS, LAYER_BRM_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::designPathConstraintsOfOut(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerOut, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisc, int disc1, int disc2)
{
	auto inner = layerILM->getYs();
	auto outer = layerOut->getYs();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	const int movesMin = (isDisc ? PATH_OPR_MOVES_MIN_DISC : PATH_OPR_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_OPR_MOVES_MAX_DISC : PATH_OPR_MOVES_MAX);

	if (isDisc)
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(inner[i] + (outer[i] - inner[i]) / 5, inner[i]+3); // max(outer[i] + 0, 0); // inner[i]);
			upper[i] = max(min(upper[i], outer[i]), 0);
			lower[i] = min(outer[i] + 24, height - 1);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(inner[i] + (outer[i] - inner[i]) / 5, inner[i] + 3); // max(outer[i] + 0, 0); // inner[i]);
			upper[i] = max(min(upper[i], outer[i]), 0);
			lower[i] = min(outer[i] + 24, height - 1);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	return true;
}


bool RetSegm::OptimalPath::designPathConstraintsOfILM(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut,
													std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, 
													int reti1, int reti2, bool isDisk, int disc1, int disc2)
{
	auto inner = layerInn->getYs();
	auto outer = layerOut->getYs();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	bool found = (isDisk && (disc2 - disc1) > 1);
	int upperSpan = (found ? PATH_ILM_UPPER_SPAN_DISC : PATH_ILM_UPPER_SPAN);
	int lowerSpan = (found ? PATH_ILM_LOWER_SPAN_DISC : PATH_ILM_LOWER_SPAN);
	int movesMin = (found ? PATH_ILM_MOVES_MIN_DISC : PATH_ILM_MOVES_MIN);
	int movesMax = (found ? PATH_ILM_MOVES_MAX_DISC : PATH_ILM_MOVES_MAX);

	if (isDisk) 
	{
		int moves = 0;
		for (int i = 0; i < size; i++) {
			if (i < (size - 1)) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}

			if (i < reti1 || i > reti2) {
				upper[i] = inner[i] - upperSpan;
				lower[i] = (inner[i] + outer[i]) / 2;

			}
			else if (i >= disc1 && i <= disc2) {
				if (inner[i] > outer[i]) {
					upper[i] = inner[i] - upperSpan;
					lower[i] = inner[i] + lowerSpan;
				}
				else {
					upper[i] = inner[i] - upperSpan;
					lower[i] = inner[i] + 125;
					// moves = movesMax;
				}
				moves *= 2;
			}
			else {
				upper[i] = inner[i] - upperSpan;
				lower[i] = (inner[i] + outer[i]) / 2;
			}

			delta[i] = min(max(moves, movesMin), movesMax);
			upper[i] = min(max(upper[i], 0), height - 1);
			lower[i] = min(max(lower[i], 0), height - 1);
		}

		/*
		if (found) {
			int width = disc2 - disc1 + 1;
			int peri_x1 = max(disc1 - width / 2, 0);
			int peri_x2 = min(disc2 + width / 2, size - 1);
			auto iter_px1 = inner.cbegin() + peri_x1;
			auto iter_px2 = inner.cbegin() + peri_x2;

			auto minmax = std::minmax_element(iter_px1, iter_px2);
			int top_y = *minmax.first;
			int bot_y = *minmax.second;
			int mid_y = (bot_y + top_y) / 2;
			auto iter_bot = minmax.second;

			auto deep_x1 = peri_x1;
			auto deep_x2 = peri_x2;
			for (auto iter = iter_px1; iter <= iter_bot; iter++) {
				if (*iter >= mid_y) {
					deep_x1 = (int)(iter - inner.begin());
					break;
				}
			}
			for (auto iter = iter_px2; iter >= iter_bot; iter--) {
				if (*iter >= mid_y) {
					deep_x2 = (int)(iter - inner.begin());
					break;
				}
			}

			int moves = 0;
			for (int i = 0; i < size; i++) {
				if (i < (size - 1)) {
					moves = (int)(abs(inner[i + 1] - inner[i]));
				}

				if (i >= deep_x1 && i <= deep_x2) {
					upper[i] = max(top_y, 0);
					lower[i] = min(inner[i] + 35, height - 1);
					delta[i] = min(max(moves, PATH_ILM_MOVES_MIN_DISK), PATH_ILM_MOVES_MAX_DISK);
				}
				else if (i >= peri_x1 && i <= peri_x2) {
					upper[i] = max(top_y - 15, 0);
					lower[i] = min(inner[i] + 5, height - 1);
					delta[i] = min(max(moves, PATH_ILM_MOVES_MIN_DISK), PATH_ILM_MOVES_MAX_DISK);
				}
				else {
					upper[i] = max(inner[i] - upperSpan, 0);
					lower[i] = min((inner[i] + outer[i]) / 2, height - 1);
					delta[i] = min(max(moves, movesMin), movesMax);
				}

				if (i >= disc1 && i <= disc2) {
					delta[i] = max((bot_y - top_y)/4, delta[i]);
				}
			}
		}
		else {
			int moves = 0;
			for (int i = 0; i < size; i++) {
				upper[i] = inner[i] - upperSpan;
				lower[i] = (inner[i] + outer[i]) / 2;
				upper[i] = max(upper[i], 0);
				lower[i] = min(min(lower[i], outer[i]), height - 1);

				if (i < (size - 1)) {
					moves = abs(inner[i + 1] - inner[i]);
				}
				delta[i] = min(max(moves, movesMin), movesMax);
			}
		}
		*/
	}
	else {
		int moves = 0;
		for (int i = 0; i < size; i++) {
			if (i < reti1 || i > reti2) {
				upper[i] = inner[i] - upperSpan;
				lower[i] = (inner[i] + outer[i]) / 2;
			}
			else {
				upper[i] = inner[i] - upperSpan;
				lower[i] = (inner[i] + outer[i]) / 2;
			}

			if (i < (size - 1)) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}

			delta[i] = min(max(moves, movesMin), movesMax);
			upper[i] = min(max(upper[i], 0), height - 1);
			lower[i] = min(max(lower[i], 0), height - 1);
		}

		/*
		int moves = 0;
		for (int i = 0; i < size; i++) {
			upper[i] = inner[i] - upperSpan;
			lower[i] = (inner[i] + outer[i]) / 2;
			upper[i] = max(upper[i], 0);
			lower[i] = min(min(lower[i], outer[i]), height-1);

			if (i < (size - 1)) {
				moves = abs(inner[i + 1] - inner[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
		*/
	}
	return true;
}


bool RetSegm::OptimalPath::designPathConstraintsOfOPR(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerOut,
														std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, 
														bool isDisc, int disc1, int disc2)
{
	auto inner = layerILM->getYs();
	auto outer = layerOut->getYs();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	const int movesMin = (isDisc ? PATH_OPR_MOVES_MIN_DISC : PATH_OPR_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_OPR_MOVES_MAX_DISC : PATH_OPR_MOVES_MAX);

	if (isDisc)
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(outer[i] + 3, inner[i]); // max(outer[i] + 0, 0); // inner[i]);
			lower[i] = min(outer[i] + 18, height - 1);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(outer[i] + 3, inner[i]); // max(outer[i] + 0, 0); // inner[i]);
			lower[i] = min(outer[i] + 18, height - 1);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	return true;


	if (isDisc) 
	{
		/*
		// Optic disc area enclosing its raised nerves. 
		bool found = (isDisc && (disc2 - disc1) > 1);

		int dpos1 = -1, dpos2 = -1;
		bool found = false;
		auto xdist = vector<int>(size, 0);

		int disc_w = disc2 - disc1 + 1;
		if (disc_w > 1) {
			int disc_r = min(disc_w / 5, 12);
			dpos1 = max(disc1 - disc_r, 0);
			dpos2 = min(disc2 + disc_r, size - 1);
			found = true;

			for (int c = 0; c < size; c++) {
				if (c <= dpos1) {
					xdist[c] = dpos1 - c + 1;
				}
				else if (c >= dpos2) {
					xdist[c] = c - dpos2 + 1;
				}
			}
		}

		int bottomY = 0;
		for (int i = 0; i < size; i++) {
			bottomY = max(inner[i], bottomY);
		}
		bottomY += (PATH_OPR_LOWER_SPAN_DISC / 2);
		bottomY = min(bottomY, height - 1);

		int moves;
		for (int i = 0; i < size; i++) {
			if (i >= dpos1 && i <= dpos2) {
				upper[i] = max(outer[i], inner[i]);
				if (i >= disc1 && i <= disc2) {
					// lower[i] = min(max(outer[i] + PATH_OPR_LOWER_SPAN_DISC, bottomY), height - 1);
					lower[i] = min(min(upper[i] + PATH_OPR_LOWER_SPAN_DISC, bottomY), height - 1);
				}
				else {
					lower[i] = min(upper[i] + PATH_OPR_LOWER_SPAN/2, height - 1);
				}
			}
			else {
				upper[i] = max(outer[i] - PATH_OPR_UPPER_SPAN, inner[i]);
				lower[i] = min(outer[i] + PATH_OPR_LOWER_SPAN, height - 1);
			}

			if (i < size - 1) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
		*/
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(outer[i] + PATH_OPR_UPPER_SPAN, 0); // inner[i]);
			lower[i] = min(outer[i] + PATH_OPR_LOWER_SPAN, height - 1);

			if (i < size - 1) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else 
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(outer[i] + PATH_OPR_UPPER_SPAN, inner[i]);
			lower[i] = min(outer[i] + PATH_OPR_LOWER_SPAN, height - 1);

			if (i < size - 1) {
				moves = abs(inner[i + 1] - inner[i]) + 1;
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	return true;
}



bool RetSegm::OptimalPath::designPathConstraintsOfIOS(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerOut, SegmLayer * layerOPR,
														std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, 
														bool isDisc, int disc1, int disc2)
{
	auto inner = layerILM->getYs();
	auto basel = layerOut->getYs();
	auto outer = layerOPR->getYs();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();


	if (size <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(size, 0);
	lower = vector<int>(size, height - 1);
	delta = vector<int>(size, PATH_IOS_MOVES_MIN);

	const int movesMin = (isDisc ? PATH_IOS_MOVES_MIN_DISC : PATH_IOS_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_IOS_MOVES_MAX_DISC : PATH_IOS_MOVES_MAX);

	if (isDisc)
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(basel[i], inner[i]);
			lower[i] = min(basel[i] + 10, outer[i]);
			upper[i] = min(upper[i], lower[i]);

			if (i >= disc1 && i <= disc2) {
				upper[i] = max(upper[i], outer[i] - 12);
				lower[i] = max(lower[i], outer[i]);
			}

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(basel[i], inner[i]);
			lower[i] = min(basel[i] + 10, outer[i]);
			upper[i] = min(upper[i], lower[i]);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}

	/*
	if (isDisc)
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(outer[i] - 12, 0); // inner[i]);
			lower[i] = min(outer[i] + 0, height - 1);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(outer[i] - 12, inner[i]);
			lower[i] = min(outer[i] + 0, height - 1);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	*/
	return true;

	if (isDisc)
	{
		/*
		// Optic disc area enclosing its raised nerves. 
		int dpos1 = -1, dpos2 = -1;
		bool found = false;
		auto xdist = vector<int>(size, 0);

		int disc_w = disc2 - disc1 + 1;
		if (disc_w > 1) {
			int disc_r = min(disc_w / 4, 12);
			dpos1 = max(disc1 - disc_r, 0);
			dpos2 = min(disc2 + disc_r, size - 1);
			found = true;

			for (int c = 0; c < size; c++) {
				if (c <= dpos1) {
					xdist[c] = dpos1 - c + 1;
				}
				else if (c >= dpos2) {
					xdist[c] = c - dpos2 + 1;
				}
			}
		}

		int moves;
		for (int i = 0; i < size; i++) {
			if (i >= dpos1 && i <= dpos2) {
				upper[i] = max(outer[i] - PATH_IOS_UPPER_SPAN_DISC, inner[i]);
				upper[i] = max(upper[i], basel[i]);
				lower[i] = min(upper[i] + PATH_IOS_LOWER_SPAN_DISC, height - 1);
			}
			else {
				upper[i] = max(outer[i] - PATH_IOS_UPPER_SPAN, inner[i]);
				lower[i] = min(outer[i] + PATH_IOS_LOWER_SPAN, height - 1);
				// lower[i] = min(outer[i], height - 1);
			}

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]) + 1;
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
		*/
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(outer[i] - PATH_IOS_UPPER_SPAN, 0); // inner[i]);
			lower[i] = min(outer[i] + PATH_IOS_LOWER_SPAN, height - 1);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(outer[i] - PATH_IOS_UPPER_SPAN, inner[i]);
			lower[i] = min(outer[i] + PATH_IOS_LOWER_SPAN, height - 1);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}

	return true;
}


bool RetSegm::OptimalPath::designPathConstraintsOfBRM(const SegmImage * imgSrc, SegmLayer * layerIOS, SegmLayer * layerOPR,
														std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
														bool isDisc, int disc1, int disc2)
{
	auto inner = layerIOS->getYs();
	auto outer = layerOPR->getYs();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	const int movesMin = (isDisc ? PATH_BRM_MOVES_MIN_DISC : PATH_BRM_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_BRM_MOVES_MAX_DISC : PATH_BRM_MOVES_MAX);

	if (isDisc)
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = outer[i]; // max(outer[i], inner[i]); // min(outer[i] + 0, inner[i] + (outer[i] - inner[i]) / 3);
			lower[i] = outer[i] + 6; // min(outer[i] + 6, inner[i] + 14);
			//upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			upper[i] = min(upper[i], lower[i]);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = outer[i]; // max(outer[i], inner[i]); // min(outer[i] + 0, inner[i] + (outer[i] - inner[i]) / 3);
			lower[i] = outer[i] + 6; // min(outer[i] + 8, inner[i] + 14);
			//upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);
			upper[i] = min(upper[i], lower[i]);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	return true;
	
	/*
	if (isDisc)
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(outer[i] + PATH_BRM_UPPER_SPAN, inner[i] + 7);
			lower[i] = min(outer[i] + PATH_BRM_LOWER_SPAN, inner[i] + 14);
			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);

			if (i < size - 1) {
				moves = abs(inner[i + 1] - inner[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(outer[i] + PATH_BRM_UPPER_SPAN, inner[i] + 7);
			lower[i] = min(outer[i] + PATH_BRM_LOWER_SPAN, inner[i] + 14);
			upper[i] = max(upper[i], 0);
			lower[i] = min(lower[i], height - 1);

			if (i < size - 1) {
				moves = abs(inner[i + 1] - inner[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	*/
	return true;
}


bool RetSegm::OptimalPath::designPathConstraintsOfRPE(const SegmImage * imgSrc, SegmLayer * layerIOS, SegmLayer* layerOPR, SegmLayer * layerBRM, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisc, int disc1, int disc2)
{
	auto inner = layerIOS->getYs();
	auto layer = layerOPR->getYs();
	auto outer = layerBRM->getYs();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	const int movesMin = (isDisc ? PATH_RPE_MOVES_MIN_DISC : PATH_RPE_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_RPE_MOVES_MAX_DISC : PATH_RPE_MOVES_MAX);

	if (isDisc)
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(layer[i], inner[i] + (layer[i] - inner[i]) / 2);
			lower[i] = min(layer[i] + (outer[i]- layer[i])/2, height-1);
			upper[i] = min(upper[i], lower[i]);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(layer[i], inner[i] + (layer[i] - inner[i]) / 2);
			lower[i] = min(layer[i] + (outer[i] - layer[i]) / 2, height - 1);
			upper[i] = min(upper[i], lower[i]);

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	return true;

	/*
	delta = basel;
	Mat srcMat = imgSrc->getCvMatConst();
	for (int i = 0; i < inner.size(); i++) {
		int y1 = max(inner[i], basel[i] - 3);
		int y2 = min(outer[i], basel[i] + 3);

		int maxIdx = 0, maxVal = 0;
		int curVal = 0;
		for (int r = (y1 + 1); r <= (y2 - 1); r++) {
			curVal = srcMat.at<uint8_t>(r, i) - srcMat.at<uint8_t>(r - 1, i);
			if (curVal > maxVal) {
				maxVal = curVal;
				maxIdx = r;
			}
		}

		if (maxIdx > 0) {
			delta[i] = maxIdx;
		}
	}
	*/

	/*
	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	const int movesMin = (isDisc ? PATH_RPE_MOVES_MIN_DISC : PATH_RPE_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_RPE_MOVES_MAX_DISC : PATH_RPE_MOVES_MAX);


	int moves;
	for (int i = 0; i < size; i++) {
		upper[i] = max(outer[i] - PATH_RPE_UPPER_SPAN, 0);
		lower[i] = min(outer[i] - PATH_RPE_LOWER_SPAN, height - 1);

		if (i < size - 1) {
			moves = abs(inner[i + 1] - inner[i]);
		}
		delta[i] = min(max(moves, movesMin), movesMax);
	}
	*/

	/*
	if (isDisc)
	{
		int moves;
		for (int i = 0; i < size; i++) {
			if (i >= disc1 && i <= disc2) {
				upper[i] = inner[i];
				lower[i] = outer[i];
			}
			else {
				upper[i] = max(basel[i]-5, inner[i]);
				lower[i] = outer[i];
			}

			if (i < size - 1) {
				moves = abs(inner[i + 1] - inner[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else
	{
		int moves;
		for (int i = 0; i < size; i++) {
			upper[i] = max(basel[i]-5, inner[i]);
			lower[i] = outer[i];

			if (i < size - 1) {
				moves = abs(inner[i + 1] - inner[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	*/
	return true;
}


bool RetSegm::OptimalPath::designPathConstraintsOfOPL(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIOS, 
														std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
														bool isDisc, int disc1, int disc2)		
{
	auto inner = layerILM->getYs();
	auto outer = layerIOS->getYs();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	const int foveaSize = 20;
	const int foveaDepth = 10;

	int fovea = -1;
	int score = 0;
	for (int i = foveaSize; i < (size - foveaSize); i++) {
		if (isDisc && i >= disc1 && i <= disc2) {
			continue;
		}

		int depth1 = 0, depth2 = 0;
		for (int j = (i - foveaSize); j < i; j++) {
			depth1 = max(depth1, (inner[i] - inner[j]));
		}
		for (int j = i + 1; j < (i + foveaSize); j++) {
			depth2 = max(depth2, (inner[i] - inner[j]));
		}
		if (depth1 >= foveaDepth && depth2 >= foveaDepth) {
			if (score < (depth1 + depth2) / 2) {
				score = (depth1 + depth2) / 2;
				fovea = i;
			}
		}
	}

	int freg1 = (fovea >= 0 ? fovea - foveaSize : -1);
	int freg2 = (fovea >= 0 ? fovea + foveaSize : -1);

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	const int movesMin = (isDisc ? PATH_OPL_MOVES_MIN_DISC : PATH_OPL_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_OPL_MOVES_MAX_DISC : PATH_OPL_MOVES_MAX);

	if (isDisc)
	{
		int moves;
		for (int i = 0; i < size; i++) {
			if (i >= freg1 && i <= freg2) {
				upper[i] = max(outer[i] - 24, inner[i]);
				lower[i] = min(outer[i] + 0, height - 1);
			}
			else {
				upper[i] = max((int)(outer[i] - (outer[i] - inner[i]) * 0.45f), inner[i]);
				lower[i] = min(outer[i] + 0, height - 1);
			}

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	else
	{
		int moves;
		for (int i = 0; i < size; i++) {
			if (i >= freg1 && i <= freg2) {
				upper[i] = max(outer[i] - 24, inner[i]);
				lower[i] = min(outer[i] + 0, height - 1);
			}
			else {
				upper[i] = max((int)(outer[i] - (outer[i] - inner[i]) * 0.45f), inner[i]);
				lower[i] = min(outer[i] + 0, height - 1);
			}

			if (i < size - 1) {
				moves = abs(outer[i + 1] - outer[i]);
			}
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	return true;


	SegmImage imgOut;
	Mat srcMat = imgSrc->getCvMatConst();

	if (isDisc)
	{
		bool found = (isDisc && (disc2 - disc1) > 1);
		int width = disc2 - disc1 + 1;
		int peri1 = max(disc1 - width / 3, 0);
		int peri2 = min(disc2 + width / 3, size - 1);
		int min_thresh = (int)(imgSrc->getMean() + imgSrc->getStddev());

		if (found) {
			int moves, range;
			for (int i = 0; i < size; i++) {
				if (i >= peri1 && i <= peri2) {
					range = min((int)((outer[i] - inner[i])*0.25f), PATH_OPL_UPPER_SPAN);
					upper[i] = max(outer[i] - range, inner[i]);
					range = min((int)((outer[i] - inner[i])*0.00f), PATH_OPL_LOWER_SPAN);
					lower[i] = max(outer[i] - range, upper[i]);
				}
				else {
					range = min((int)((outer[i] - inner[i])*0.85f), PATH_OPL_UPPER_SPAN);
					upper[i] = max(outer[i] - range, inner[i]);
					range = max((int)((outer[i] - inner[i])*0.15f), PATH_OPL_LOWER_SPAN);
					lower[i] = max(outer[i] - range, upper[i]);
				}

				moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i]) : moves);
				delta[i] = min(max(moves, movesMin), movesMax);
			}

			Coarse::createAverageMap(imgSrc, &imgOut, 7, 7);
			Mat outMat = imgOut.getCvMatConst();

			for (int c = 0; c < size; c++) {
				int min_val = 255;
				int min_pos = upper[c];
				int cur_val = 0, thresh = 255;

				for (int r = lower[c]; r >= upper[c]; r--) {
					cur_val = outMat.at<uint8_t>(r, c);
					if (cur_val < min_val) {
						min_val = outMat.at<uint8_t>(r, c);
						min_pos = r;
						thresh = (int)(min_val * 1.35f);
					}
					else if (cur_val >= thresh) {
						//break;
					}
				}

				if (min_val > min_thresh) {
					upper[c] = max(upper[c], lower[c] - 5);
				}
				else {
					upper[c] = max(min_pos - 7, inner[c]);
					lower[c] = min_pos;
				}
			}
		}
		else {
			int moves, range;
			for (int i = 0; i < size; i++) {
				range = min((int)((outer[i] - inner[i])*0.85f), PATH_OPL_UPPER_SPAN);
				upper[i] = max(outer[i] - range, inner[i]);
				range = max((int)((outer[i] - inner[i])*0.15f), PATH_OPL_LOWER_SPAN);
				lower[i] = max(outer[i] - range, upper[i]);

				moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i]) : moves);
				delta[i] = min(max(moves, movesMin), movesMax);
			}

			Coarse::createAverageMap(imgSrc, &imgOut, 7, 7);
			Mat outMat = imgOut.getCvMatConst();

			for (int c = 0; c < size; c++) {
				int min_val = 255;
				int min_pos = upper[c];
				int cur_val = 0, thresh = 255;

				for (int r = lower[c]; r >= upper[c]; r--) {
					cur_val = outMat.at<uint8_t>(r, c);
					if (cur_val < min_val) {
						min_val = outMat.at<uint8_t>(r, c);
						min_pos = r;
						thresh = (int)(min_val * 1.35f);
					}
					else if (cur_val >= thresh) {
						//break;
					}
				}

				if (min_val > min_thresh) {
					upper[c] = max(upper[c], lower[c] - 5);
				}
				else {
					upper[c] = max(min_pos - 7, inner[c]);
					lower[c] = min_pos;
				}
			}
		}

		/*
		// Optic disc area enclosing its raised nerves. 
		int dpos1 = -1, dpos2 = -1;
		bool found = false;

		int disc_w = disc2 - disc1 + 1;
		if (disc_w > 1) {
			int disc_r = max(disc_w, 24) / 4;
			dpos1 = max(disc1 - disc_r, 0);
			dpos2 = min(disc2 + disc_r, size - 1);
			found = true;
		}

		int moves, range;
		for (int i = 0; i < size; i++) {
			if (i >= dpos1 && i <= dpos2) {
				range = min((int)((outer[i] - inner[i])*0.35f), PATH_OPL_UPPER_SPAN_DISC);
				upper[i] = max(outer[i] - range, inner[i]);
				range = max((int)((outer[i] - inner[i])*0.05f), PATH_OPL_LOWER_SPAN_DISC);
				lower[i] = max(outer[i] - range, upper[i]);
			}
			else {
				range = min((int)((outer[i] - inner[i])*0.85f), PATH_OPL_UPPER_SPAN);
				upper[i] = max(outer[i] - range, inner[i]);
				range = max((int)((outer[i] - inner[i])*0.10f), PATH_OPL_LOWER_SPAN);
				lower[i] = max(outer[i] - range, upper[i]);
			}

			moves = (i < (size - 1) ? abs(outer[i + 1] - outer[i] + 1) : moves);
			delta[i] = min(max(moves, movesMin), movesMax);
		}

		Coarse::createAverageMap(imgSrc, &imgOut, 7, 7);
		Mat outMat = imgOut.getCvMatConst();

		for (int c = 0; c < size; c++) {
			int min_val = 255;
			int min_pos = upper[c];
			int cur_val = 0, thresh = 255;
			
			for (int r = lower[c]; r >= upper[c]; r--) {
				cur_val = outMat.at<uint8_t>(r, c);
				if (cur_val < min_val) {
					min_val = outMat.at<uint8_t>(r, c);
					min_pos = r;
					thresh = (int)(min_val * 1.35f);
				}
				else if (cur_val >= thresh) {
					//break;
				}
			}
			upper[c] = max(min_pos - 7, upper[c]);
			lower[c] = min_pos;
		}
		*/
	}
	else
	{
		int moves, range;
		for (int i = 0; i < size; i++) {
			range = min((int)((outer[i] - inner[i])*0.85f), PATH_OPL_UPPER_SPAN);
			upper[i] = max(outer[i] - range, inner[i]);
			range = max((int)((outer[i] - inner[i])*0.15f), PATH_OPL_LOWER_SPAN);
			lower[i] = max(outer[i] - range, upper[i]);

			moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i]) : moves);
			delta[i] = min(max(moves, movesMin), movesMax);
		}

		Coarse::createAverageMap(imgSrc, &imgOut, 7, 7);
		Mat outMat = imgOut.getCvMatConst();

		for (int c = 0; c < size; c++) {
			int min_val = 255;
			int min_pos = upper[c];
			int cur_val = 0, thresh = 255;

			for (int r = lower[c]; r >= upper[c]; r--) {
				cur_val = outMat.at<uint8_t>(r, c);
				if (cur_val < min_val) {
					min_val = outMat.at<uint8_t>(r, c);
					min_pos = r;
					thresh = (int)(min_val * 1.35f);
				}
				else if (cur_val >= thresh) {
					//break;
				}
			}
			upper[c] = max(min_pos - 7, inner[c]);
			lower[c] = min_pos;
		}
	}
	return true;




	if (isDisc) 
	{
		Coarse::createAverageMap(imgSrc, &imgOut, 7, 7);
		Mat outMat = imgOut.getCvMatConst();

		for (int c = 0, r = 0; c < size; c++) {
			int start = outer[c] - (int)(((outer[c] - inner[c]) * 0.15f));
			int close = outer[c] - (int)(((outer[c] - inner[c]) * 0.55f));
			int min_val = 255, min_pos = -1;

			for (r = start; r >= close; r--) {
				if (outMat.at<uint8_t>(r, c) <= min_val) {
					min_val = outMat.at<uint8_t>(r, c);
					min_pos = r;
				}
			}

			upper[c] = max(min_pos - 10, close);
			lower[c] = min(min_pos, height - 1);
			delta[c] = PATH_OPL_MOVES_MIN;

			if (c >= disc1 && c <= disc2) {
				if (c > 0) {
					delta[c] = abs(inner[c] - inner[c - 1]);
					delta[c] = min(max(delta[c], PATH_OPL_MOVES_MIN * 2), PATH_OPL_MOVES_MAX);
				}
			}
		}

		int range_y1 = height - 1;
		int range_y2 = 0;
		for (int c = 0, r = 0; c < (size / 10); c++) {
			range_y1 = min(range_y1, upper[c]);
			range_y2 = max(range_y2, lower[c]);
			break;
		}

		int moves = PATH_OPL_MOVES_MIN;
		int edges;
		for (int c = 0, r = 0; c < size; c++) {
			if (abs(range_y1 - upper[c]) > moves) {
				upper[c] = range_y1 + moves * (range_y1 < upper[c] ? +1 : -1);
			}
			if (abs(range_y2 - lower[c]) > moves) {
				lower[c] = range_y2 + moves * (range_y2 < lower[c] ? +1 : -1);
			}

			if (c >= disc1 && c <= disc2) {
				edges = min(c - disc1, disc2 - c) + 1;
				moves = min(PATH_OPL_MOVES_MAX, PATH_OPL_MOVES_MIN * edges);
			}
			else {
				moves = PATH_IPL_MOVES_MIN;
			}

			range_y1 = upper[c];
			range_y2 = lower[c];
			delta[c] = moves;
		}

		/*
		int thresh = (int)(imgSrc->getMean() + imgSrc->getStddev());

		for (int c = 0, r = 0; c < size; c++) {
			if (c >= disc1 && c <= disc2) {
				upper[c] = max(inner[c], 0);
				lower[c] = min(outer[c], height - 1);
				delta[c] = PATH_OPL_MOVES_MIN;

				if (c > 0) {
					delta[c] = abs(inner[c] - inner[c - 1]);
					delta[c] = min(max(delta[c], PATH_OPL_MOVES_MIN * 2), PATH_OPL_MOVES_MAX);
				}
			}
			else {
				int start = outer[c] - (int)(((outer[c] - inner[c]) * 0.15f));
				int close = inner[c] + (int)(((outer[c] - inner[c]) * 0.50f));

				for (r = start; r >= close; r--) {
					if (outMat.at<uint8_t>(r, c) >= thresh) {
						break;
					}
				}
				for (; r >= close; r--) {
					if (outMat.at<uint8_t>(r, c) <= (thresh / 3)) {
						break;
					}
				}

				upper[c] = max(r, inner[c]);
				lower[c] = min(start, height - 1);
				delta[c] = PATH_OPL_MOVES_MIN;

				// int start = outer[c] - (int)(((outer[c] - inner[c]) * 0.15f));
				// int close = inner[c] + (int)(((outer[c] - inner[c]) * 0.50f));

				upper[c] = max(r, inner[c]);
				lower[c] = upper[c];
				delta[c] = PATH_OPL_MOVES_MAX * 4; //  PATH_OPL_MOVES_MIN;
			}
		}
		*/
		/*
		for (int i = 0; i < size; i++) {
			if (i >= disc1 && i <= disc2) {
				upper[i] = max(inner[i], 0);
				lower[i] = min(outer[i], height - 1);
				delta[i] = PATH_OPL_MOVES_MIN;

				if (i > 0) {
					delta[i] = abs(inner[i] - inner[i - 1]);
					delta[i] = min(max(delta[i], PATH_OPL_MOVES_MIN), PATH_OPL_MOVES_MAX);
				}
			}
			else {
				upper[i] = max(inner[i] + (int)(((outer[i] - inner[i]) * 0.55f)), 0);
				lower[i] = min(outer[i] - (int)(((outer[i] - inner[i]) * 0.15f)), height - 1);
				delta[i] = PATH_OPL_MOVES_MIN;
			}
		}
		*/
		/*
		Coarse::createGraidentMap(imgSrc, &imgOut, 7, 7, true);
		Mat outMat = imgOut.getCvMatConst();

		int thresh = (int)(imgSrc->getMean() + imgSrc->getStddev());

		for (int c = 0, r = 0; c < size; c++) {
			if (c >= disc1 && c <= disc2) {
				upper[c] = max(inner[c], 0);
				lower[c] = min(outer[c], height - 1);
				delta[c] = PATH_OPL_MOVES_MIN;

				if (c > 0) {
					delta[c] = abs(inner[c] - inner[c - 1]);
					delta[c] = min(max(delta[c], PATH_OPL_MOVES_MIN*2), PATH_OPL_MOVES_MAX);
				}
			}
			else {
				int start = outer[c] - (int)(((outer[c] - inner[c]) * 0.15f));
				int close = inner[c] + (int)(((outer[c] - inner[c]) * 0.50f));

				for (r = start; r >= close; r--) {
					if (outMat.at<uint8_t>(r, c) >= thresh) {
						break;
					}
				}
				for (; r >= close; r--) {
					if (outMat.at<uint8_t>(r, c) <= (thresh / 3)) {
						break;
					}
				}

				upper[c] = max(r, inner[c]);
				lower[c] = min(start, height - 1);
				delta[c] = PATH_OPL_MOVES_MIN;
			
				// int start = outer[c] - (int)(((outer[c] - inner[c]) * 0.15f));
				// int close = inner[c] + (int)(((outer[c] - inner[c]) * 0.50f));

				upper[c] = max(r, inner[c]);
				lower[c] = upper[c];
				delta[c] = PATH_OPL_MOVES_MAX * 4; //  PATH_OPL_MOVES_MIN;
			}
		}
		*/
	}
	else {

		Coarse::createGraidentMap(imgSrc, &imgOut, 7, 7, true);
		Mat outMat = imgOut.getCvMatConst();

		/*
		for (int i = 0; i < size; i++) {
			upper[i] = max(inner[i] + (int)(((outer[i] - inner[i]) * 0.15f)), 0);
			lower[i] = min(outer[i] - (int)(((outer[i] - inner[i]) * 0.05f)), height - 1);
			delta[i] = PATH_OPL_MOVES_MIN;
		}
		*/

		int thresh = (int)(imgSrc->getMean() + imgSrc->getStddev());

		for (int c = 0, r = 0; c < size; c++) {
			int start = outer[c] - (int)(((outer[c] - inner[c]) * 0.15f));

			for (r = start; r >= inner[c]; r--) {
				if (outMat.at<uint8_t>(r, c) >= thresh) {
					break;
				}
			}
			for (; r >= inner[c]; r--) {
				if (outMat.at<uint8_t>(r, c) <= (thresh/3)) {
					break;
				}
			}

			upper[c] = max(r, inner[c]);
			lower[c] = min(start, height - 1);
			delta[c] = PATH_OPL_MOVES_MIN;
		}

	}
	return true;
}


bool RetSegm::OptimalPath::designPathConstraintsOfIPL(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerOPL, 
														std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
														bool isDisc, int disc1, int disc2)
{
	auto inner = layerILM->getYs();
	auto outer = layerOPL->getYs();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	SegmImage imgOut;
	Coarse::createAverageMap(imgSrc, &imgOut, 5, 5);
	Mat outMat = imgOut.getCvMatConst();

	// Peak values around ILM and RPE layer. 
	auto inn_peak_vals = vector<int>(size, 0);
	auto out_peak_vals = vector<int>(size, 0);
	auto out_peak_tops = vector<int>(size, 0);
	auto inn_peak_locs = upper;
	auto out_peak_locs = lower;

	int start, close;
	int gsum = 0, gcnt = 0;
	for (int c = 0, r = 0; c < size; c++) {
		start = upper[c];
		close = lower[c];
		gsum = 0, gcnt = 0;
		for (r = start; r < close; r++) {
			if (r < 0 || r >= height) {
				continue;
			}
			if (out_peak_tops[c] <= outMat.at<uint8_t>(r, c)) {
				out_peak_tops[c] = outMat.at<uint8_t>(r, c);
				out_peak_locs[c] = r;
			}
			gsum += outMat.at<uint8_t>(r, c);
			gcnt++;
		}
		if (gcnt > 0) {
			out_peak_vals[c] = gsum / gcnt;
		}

		start = inner[c];
		close = inner[c] + (outer[c] - inner[c]) / (isDisc ? 2 : 1);
		gsum = gcnt = 0;
		for (r = start; r < close; r++) {
			if (r < 0 || r >= height) {
				continue;
			}
			gsum += outMat.at<uint8_t>(r, c);
			gcnt++;
		}
		if (gcnt > 0) {
			inn_peak_vals[c] = gsum / gcnt;
		}
	}

	int out_peak_mean = 0;
	int out_tops_mean = 0;

	for (int i = 0; i < size; i++) {
		out_peak_mean += out_peak_vals[i];
		out_tops_mean += out_peak_tops[i];
	}
	out_peak_mean = out_peak_mean / size;
	out_tops_mean = out_tops_mean / size;

	int thresh, threshMax, threshMin;
	threshMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * 4.0f);
	threshMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * 1.0f);

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	const int movesMin = (isDisc ? PATH_IPL_MOVES_MIN_DISC : PATH_IPL_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_IPL_MOVES_MAX_DISC : PATH_IPL_MOVES_MAX);

	if (isDisc)
	{
		// Optic disc area enclosing its raised nerves. 
		int dpos1 = -1, dpos2 = -1;
		bool found = false;

		int disc_w = disc2 - disc1 + 1;
		if (disc_w > 1) {
			int disc_r = max(disc_w, 24) / 5;
			dpos1 = max(disc1 - disc_r, 0);
			dpos2 = min(disc2 + disc_r, size - 1);
			found = true;
		}

		int moves, range;
		if (found) {
			for (int i = 0; i < size; i++) {
				if (i >= dpos1 && i <= dpos2) {
					range = min((int)((outer[i] - inner[i])*0.25f), PATH_IPL_UPPER_SPAN_DISC/2);
					upper[i] = max(outer[i] - range, inner[i]);
					range = max((int)((outer[i] - inner[i])*0.05f), PATH_IPL_LOWER_SPAN_DISC);
					lower[i] = max(outer[i] - range, upper[i]);
				}
				else {
					thresh = max(out_peak_vals[i], out_peak_mean);
					thresh = (int)(thresh * 0.70f);
					thresh = min(max(thresh, threshMin), threshMax);
					float ratio1 = (inn_peak_vals[i] < thresh ? 0.65f : 0.45f);
					float ratio2 = (inn_peak_vals[i] < thresh ? 0.25f : 0.15f);
					int limit1 = (inn_peak_vals[i] < thresh ? (PATH_IPL_UPPER_SPAN_DISC * 3) / 2 : PATH_IPL_UPPER_SPAN_DISC);

					range = min((int)((outer[i] - inner[i])*ratio1), limit1);
					upper[i] = max(outer[i] - range, inner[i]);
					range = max((int)((outer[i] - inner[i])*ratio2), PATH_IPL_LOWER_SPAN_DISC);
					lower[i] = max(outer[i] - range, upper[i]);
				}

				moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i] + 1) : moves);
				delta[i] = min(max(moves, movesMin), movesMax);
			}
		}
		else {
			for (int i = 0; i < size; i++) {
				thresh = max(out_peak_vals[i], out_peak_mean);
				thresh = (int)(thresh * 0.75f);
				thresh = min(max(thresh, threshMin), threshMax);
				float ratio1 = (inn_peak_vals[i] < thresh ? 0.65f : 0.35f);
				float ratio2 = (inn_peak_vals[i] < thresh ? 0.25f : 0.05f);
				int limit1 = (inn_peak_vals[i] < thresh ? PATH_IPL_UPPER_SPAN_DISC : PATH_IPL_UPPER_SPAN_DISC);

				range = min((int)((outer[i] - inner[i])*ratio1), limit1);
				upper[i] = max(outer[i] - range, inner[i]);
				range = max((int)((outer[i] - inner[i])*ratio2), PATH_IPL_LOWER_SPAN_DISC);
				lower[i] = max(outer[i] - range, upper[i]);
				moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i] + 1) : moves);
				delta[i] = min(max(moves, movesMin), movesMax);
				// LogD() << i << ": " << inn_peak_vals[i] << ", " << out_peak_vals[i] << " => " << thresh << ", " << upper[i] << ", " << lower[i];
			}
		}
	}
	else
	{
		int moves, range;
		for (int i = 0; i < size; i++) {
			thresh = max(out_peak_tops[i], out_peak_mean);
			thresh = int(thresh * 0.60f);
			thresh = min(max(thresh, threshMin), threshMax);

			float ratio1 = (inn_peak_vals[i] < thresh ? 0.45f : 0.35f);
			float ratio2 = (inn_peak_vals[i] < thresh ? 0.25f : 0.15f);
			int limit1 = (inn_peak_vals[i] < thresh ? PATH_IPL_LOWER_SPAN : PATH_IPL_LOWER_SPAN);

			range = min((int)((outer[i] - inner[i])*ratio1), PATH_IPL_UPPER_SPAN);
			upper[i] = max(outer[i] - range, inner[i]);
			range = max((int)((outer[i] - inner[i])*ratio2), limit1);
			lower[i] = max(outer[i] - range, upper[i]);

			moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i] + 1) : moves);
			delta[i] = min(max(moves, movesMin), movesMax);
		}
	}
	return true;
}



bool RetSegm::OptimalPath::designPathConstraintsOfNFL(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIPL,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
	bool isDisc, int disc1, int disc2)
{
	auto inner = layerILM->getYs();
	auto outer = layerIPL->getYs();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	auto opl_layer = upper;
	auto peak_locs = lower;

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	SegmImage imgOut, imgNFL;
	Mat srcMat = imgSrc->getCvMatConst();

	Coarse::createAverageMap(imgSrc, &imgOut, 5, 5);
	Mat outMat = imgOut.getCvMatConst();

	auto peak_vals = vector<int>(size, -1);
	for (int c = 0, r = 0; c < size; c++) {
		int start = max(peak_locs[c] - 10, 0);
		int close = min(peak_locs[c] + 10, height - 1);
		for (r = start; r < close; r++) {
			if (r < 0 || r >= height) {
				continue;
			}
			if (peak_vals[c] < outMat.at<uint8_t>(r, c)) {
				peak_vals[c] = outMat.at<uint8_t>(r, c);
			}
		}
	}

	const int movesMin = (isDisc ? PATH_NFL_MOVES_MIN_DISC : PATH_NFL_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_NFL_MOVES_MAX_DISC : PATH_NFL_MOVES_MAX);

	if (isDisc)
	{
		int nfl_idx;
		int thresh, moves;
		for (int c = 0, r = 0; c < size; c++) {
			thresh = (int)(imgSrc->getMean() + imgSrc->getStddev() * 3.0f);
			if (thresh < (peak_vals[c] * 0.75f)) {
				thresh = (int)(peak_vals[c] * 0.75f);
			}

			int max_idx, max_val = 0;
			int start = max(inner[c], outer[c] - 15);
			int close = outer[c]; // -(int)(((outer[c] - inner[c]) * 0.15f));

			int count = -1; max_idx = -1; max_val = 0;
			for (r = start, count = 0; r <= close; r++, count++) {
				if (r >= outMat.rows) {
					DebugOut() << "Rows index out: " << r << ", " << outMat.rows;
				}
				if (outMat.at<uint8_t>(r, c) >= max_val) {
					max_val = outMat.at<uint8_t>(r, c);
					max_idx = r;
				}

				/*
				if (count >= ((close - start) / 2)) {
				if (max_val < thresh) {
				break;
				}
				}
				*/
			}

			if (max_val >= thresh || (start > inner[c])) {
				/*
				thresh = (int)(imgSrc->getMean() + imgSrc->getStddev() * 2.0f);
				for (r = max_idx; r <= close; r++) {
				if (outMat.at<uint8_t>(r, c) >= thresh) {
				max_val = outMat.at<uint8_t>(r, c);
				max_idx = r;
				}
				}
				*/

				nfl_idx = max_idx;
				upper[c] = max(nfl_idx, inner[c]);
				lower[c] = max(close - 3, upper[c]);
			}
			else {
				nfl_idx = start;
				upper[c] = max(nfl_idx, inner[c]);
				lower[c] = min(nfl_idx + 5, outer[c]);
			}
		}

		int range_y1 = height - 1;
		int range_y2 = 0;
		for (int c = 0, r = 0; c < (size / 10); c++) {
			range_y1 = min(range_y1, upper[c]);
			range_y2 = max(range_y2, lower[c]);
			break;
		}

		for (int c = 0, r = 0; c < size; c++) {
			if (abs(range_y1 - upper[c]) > PATH_NFL_MOVES_MIN) {
				upper[c] = range_y1 + PATH_NFL_MOVES_MIN * (range_y1 < upper[c] ? +1 : -1);
			}
			if (abs(range_y2 - lower[c]) > PATH_NFL_MOVES_MIN) {
				lower[c] = range_y2 + PATH_NFL_MOVES_MIN * (range_y2 < lower[c] ? +1 : -1);
			}
			range_y1 = upper[c];
			range_y2 = lower[c];

			if (c < size - 1) {
				moves = abs(inner[c + 1] - inner[c]);
			}
			delta[c] = min(max(moves, movesMin), movesMax);
		}
	}
	else
	{
		int nfl_idx;
		int thresh, moves;
		for (int c = 0, r = 0; c < size; c++) {
			thresh = (int)(imgSrc->getMean() + imgSrc->getStddev() * 3.0f);
			if (thresh < (peak_vals[c] * 0.90f)) {
				thresh = (int)(peak_vals[c] * 0.90f);
			}

			int max_idx, max_val = 0;
			int start = inner[c];
			int close = outer[c]; // -(int)(((outer[c] - inner[c]) * 0.15f));

			int count = -1; max_idx = -1; max_val = 0;
			for (r = start, count = 0; r <= close; r++, count++) {
				if (outMat.at<uint8_t>(r, c) >= max_val) {
					max_val = outMat.at<uint8_t>(r, c);
					max_idx = r;
				}
			}

			if (max_val >= thresh) {
				nfl_idx = max_idx;
				upper[c] = max(nfl_idx - 9, inner[c]);
				lower[c] = min(close - 3, outer[c]);
			}
			else {
				nfl_idx = start;
				upper[c] = max(nfl_idx, inner[c]);
				lower[c] = min(nfl_idx + 5, outer[c]);
			}
		}

		int range_y1 = height - 1;
		int range_y2 = 0;
		for (int c = 0, r = 0; c < (size / 10); c++) {
			range_y1 = min(range_y1, upper[c]);
			range_y2 = max(range_y2, lower[c]);
			break;
		}

		for (int c = 0, r = 0; c < size; c++) {
			if (abs(range_y1 - upper[c]) > PATH_NFL_MOVES_MIN) {
				upper[c] = range_y1 + PATH_NFL_MOVES_MIN * (range_y1 < upper[c] ? +1 : -1);
			}
			if (abs(range_y2 - lower[c]) > PATH_NFL_MOVES_MIN) {
				lower[c] = range_y2 + PATH_NFL_MOVES_MIN * (range_y2 < lower[c] ? +1 : -1);
			}
			range_y1 = upper[c];
			range_y2 = lower[c];

			if (c < size - 1) {
				moves = abs(inner[c + 1] - inner[c]);
			}
			delta[c] = min(max(moves, movesMin), movesMax);
		}
	}

	return true;
}

bool RetSegm::OptimalPath::designPathConstraintsOfNFL2(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIPL,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
	bool isDisc, int disc1, int disc2)
{
	auto inner = layerILM->getYs();
	auto outer = layerIPL->getYs();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	auto opl_layer = upper;
	auto peak_locs = lower;

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	SegmImage imgOut, imgNFL;
	Mat srcMat = imgSrc->getCvMatConst();

	Coarse::createAverageMap(imgSrc, &imgOut, 5, 5);
	Mat outMat = imgOut.getCvMatConst();

	// Peak values around RPE layer. 
	auto peak_vals = vector<int>(size, -1);
	for (int c = 0, r = 0; c < size; c++) {
		int start = max(peak_locs[c] - 10, 0);
		int close = min(peak_locs[c] + 10, height - 1);
		for (r = start; r < close; r++) {
			if (r < 0 || r >= height) {
				continue;
			}
			if (peak_vals[c] < outMat.at<uint8_t>(r, c)) {
				peak_vals[c] = outMat.at<uint8_t>(r, c);
			}
		}
	}

	const int movesMin = (isDisc ? PATH_NFL_MOVES_MIN_DISC : PATH_NFL_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_NFL_MOVES_MAX_DISC : PATH_NFL_MOVES_MAX);

	if (isDisc)
	{
		// Optic disc area enclosing its raised nerves. 
		int dpos1 = -1, dpos2 = -1;
		int epos1 = -1, epos2 = -1;
		bool found = false;

		int disc_w = disc2 - disc1 + 1;
		if (disc_w > 1) {
			int disc_r1 = max(disc_w, 48);
			int disc_r2 = max(disc_w / 4, 24);
			dpos1 = max(disc1 - disc_r1, 0);
			dpos2 = min(disc2 + disc_r1, size - 1);
			epos1 = max(disc1 - disc_r2, 0);
			epos2 = min(disc2 + disc_r2, size - 1);
			found = true;
		}

		int moves;
		/*
		int moves, range;
		if (found) {
			for (int i = 0; i < size; i++) {
				if (i >= dpos1 && i <= dpos2) {
					range = min((int)((outer[i] - inner[i])*0.75f), PATH_IPL_UPPER_SPAN_DISC);
					upper[i] = max(outer[i] - range, inner[i]);
					range = max((int)((outer[i] - inner[i])*0.35f), PATH_IPL_LOWER_SPAN_DISC);
					lower[i] = max(outer[i] - range, upper[i]);
				}
				else {
					range = min((int)((outer[i] - inner[i])*0.95f), PATH_IPL_UPPER_SPAN);
					upper[i] = max(outer[i] - range, inner[i]);
					range = max((int)((outer[i] - inner[i])*0.45f), PATH_IPL_LOWER_SPAN);
					lower[i] = max(outer[i] - range, upper[i]);
				}
				moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i] + 1) : moves);
				delta[i] = min(max(moves, movesMin), movesMax);
			}
		}
		else {
			for (int i = 0; i < size; i++) {
				range = min((int)((outer[i] - inner[i])*0.95f), PATH_IPL_UPPER_SPAN);
				upper[i] = max(outer[i] - range, inner[i]);
				range = max((int)((outer[i] - inner[i])*0.15f), PATH_IPL_LOWER_SPAN);
				lower[i] = max(outer[i] - range, upper[i]);
				moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i] + 1) : moves);
				delta[i] = min(max(moves, movesMin), movesMax);
			}
		}
		*/

		upper = inner;
		lower = outer;

		int nfl_idx;
		int thresh, threshMax, threshMin;
		for (int c = 0, r = 0; c < size; c++) {
			threshMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * 3.0f);
			threshMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * 1.0f);

			if ((c >= dpos1 && c <= dpos2)) {
				thresh = (int)(peak_vals[c] * 0.75f);
			}
			else {
				thresh = (int)(peak_vals[c] * 0.90f);
			}
			thresh = min(max(thresh, threshMin), threshMax);

			// Initial region between ILM and IPL.
			int start = upper[c];
			int close = lower[c]; 
			int max_idx, max_val = 0;

			int count = -1; max_idx = -1; max_val = 0;
			for (r = start, count = 0; r <= close; r++, count++) {
				if (outMat.at<uint8_t>(r, c) >= max_val) {
					max_val = outMat.at<uint8_t>(r, c);
					max_idx = r;
				}
			}

			if (max_val >= thresh || (c >= dpos1 && c <= dpos2)) {
				nfl_idx = max_idx;
				upper[c] = max(nfl_idx, inner[c]);
				lower[c] = min(close, outer[c]);
			}
			else {
				nfl_idx = start;
				upper[c] = max(nfl_idx, inner[c]);
				lower[c] = min(nfl_idx + 9, outer[c]);
			}
		}

		if (!found) {
			int ext_spos = (int)(size * 0.15f);
			int ext_epos = (int)(size * 0.85f);
			int offs_y1, offs_y2;

			for (int c = 0, r = 0; c < size; c++) {
				if (c >= (size - 1)) {
					delta[c] = delta[c - 1];
				}
				else {
					delta[c] = abs(inner[c + 1] - inner[c]);
				}
				delta[c] = min(max(moves, movesMin), movesMax);

				if (c < ext_spos || c > ext_epos) {
					if (c > 0) {
						if (delta[c] < abs(upper[c] - upper[c - 1])) {
							delta[c] = abs(upper[c] - upper[c - 1]) + 1;
						}
						if (delta[c] < abs(lower[c] - lower[c - 1])) {
							delta[c] = abs(lower[c] - lower[c - 1]) + 1;
						}
					}
				}
				else {
					if (abs(offs_y1 - upper[c]) > PATH_NFL_MOVES_MIN) {
						upper[c] = offs_y1 + PATH_NFL_MOVES_MIN * (offs_y1 < upper[c] ? +1 : -1);
					}
					if (abs(offs_y2 - lower[c]) > PATH_NFL_MOVES_MIN) {
						lower[c] = offs_y2 + PATH_NFL_MOVES_MIN * (offs_y2 < lower[c] ? +1 : -1);
					}
				}

				upper[c] = max(upper[c], 0);
				lower[c] = min(lower[c], height - 1);
				offs_y1 = upper[c];
				offs_y2 = lower[c];
			}
		}
	}
	else
	{
		int moves, range;
		for (int i = 0; i < size; i++) {
			range = min((int)((outer[i] - inner[i])*0.99f), PATH_IPL_UPPER_SPAN);
			upper[i] = max(outer[i] - range, inner[i]);
			range = max((int)((outer[i] - inner[i])*0.25f), PATH_IPL_LOWER_SPAN);
			lower[i] = max(outer[i] - range, upper[i]);
			moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i] + 1) : moves);
			delta[i] = min(max(moves, movesMin), movesMax);
		}

		int nfl_idx;
		int thresh, threshMax, threshMin;
		for (int c = 0, r = 0; c < size; c++) {
			threshMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * 3.0f);
			threshMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * 1.0f);

			thresh = (int)(peak_vals[c] * 0.90f);
			thresh = min(max(thresh, threshMin), threshMax);

			// Initial region between ILM and IPL.
			int start = upper[c];
			int close = lower[c]; // -(int)(((outer[c] - inner[c]) * 0.15f));
			int max_idx, max_val = 0;

			int count = -1; max_idx = -1; max_val = 0;
			for (r = start, count = 0; r <= close; r++, count++) {
				if (outMat.at<uint8_t>(r, c) >= max_val) {
					max_val = outMat.at<uint8_t>(r, c);
					max_idx = r;
				}
			}

			if (max_val >= thresh) {
				nfl_idx = max_idx;
				upper[c] = max(nfl_idx - 9, inner[c]);
				lower[c] = max(close, upper[c]);
			}
			else {
				nfl_idx = start;
				upper[c] = max(nfl_idx, inner[c]);
				lower[c] = min(nfl_idx + 5, lower[c]);
			}
		}

		int range_y1 = height - 1;
		int range_y2 = 0;
		for (int c = 0, r = 0; c < (size / 10); c++) {
			range_y1 = min(range_y1, upper[c]);
			range_y2 = max(range_y2, lower[c]);
			break;
		}

		int ext_spos = (int)(size * 0.15f);
		int ext_epos = (int)(size * 0.85f);
		int offs_y1, offs_y2;

		for (int c = 0, r = 0; c < size; c++) {
			if (c >= (size - 1)) {
				delta[c] = delta[c - 1];
			}
			else {
				delta[c] = abs(inner[c + 1] - inner[c]);
			}
			delta[c] = min(max(moves, movesMin), movesMax);

			if (c < ext_spos || c > ext_epos) {
				if (c > 0) {
					if (delta[c] < abs(upper[c] - upper[c - 1])) {
						delta[c] = abs(upper[c] - upper[c - 1]) + 1;
					}
					if (delta[c] < abs(lower[c] - lower[c - 1])) {
						delta[c] = abs(lower[c] - lower[c - 1]) + 1;
					}
				}
			}
			else {
				if (abs(offs_y1 - upper[c]) > PATH_NFL_MOVES_MIN) {
					upper[c] = offs_y1 + PATH_NFL_MOVES_MIN * (offs_y1 < upper[c] ? +1 : -1);
				}
				if (abs(offs_y2 - lower[c]) > PATH_NFL_MOVES_MIN) {
					lower[c] = offs_y2 + PATH_NFL_MOVES_MIN * (offs_y2 < lower[c] ? +1 : -1);
				}
			}
			upper[c] = max(upper[c], 0);
			lower[c] = min(lower[c], height - 1);
			offs_y1 = upper[c];
			offs_y2 = lower[c];
		}
	}

	return true;
}


bool RetSegm::OptimalPath::designPathConstraintsOfNFL3(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIPL, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisc, int disc1, int disc2, bool retry)
{
	auto inner = layerILM->getYs();
	auto outer = layerIPL->getYs();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	const int movesMin = (isDisc ? PATH_NFL_MOVES_MIN_DISC : PATH_NFL_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_NFL_MOVES_MAX_DISC : PATH_NFL_MOVES_MAX);
	
	SegmImage imgOut;
	Coarse::createAverageMap(imgSrc, &imgOut, 5, 5);
	Mat outMat = imgOut.getCvMatConst();

	// Peak values around ILM and RPE layer. 
	auto inn_peak_vals = vector<int>(size, 0);
	auto out_peak_vals = vector<int>(size, 0);
	auto out_peak_sums = vector<int>(size, 0);
	auto inn_peak_locs = upper;
	auto out_peak_locs = lower;

	for (int c = 0, r = 0; c < size; c++) {
		int start = max(lower[c] - 10, 0);
		int close = min(lower[c] + 10, height - 1);
		int gsum = 0, gcnt = 0;
		for (r = start; r < close; r++) {
			if (r < 0 || r >= height) {
				continue;
			}
			if (out_peak_vals[c] <= outMat.at<uint8_t>(r, c)) {
				out_peak_vals[c] = outMat.at<uint8_t>(r, c);
				out_peak_locs[c] = r;
			}
			gsum += outMat.at<uint8_t>(r, c);
			gcnt++;
		}

		if (gcnt > 0) {
			out_peak_sums[c] = gsum / gcnt;
		}

		start = inner[c];
		close = inner[c] + (outer[c] - inner[c]) / 2;
		for (r = start; r < close; r++) {
			if (r < 0 || r >= height) {
				continue;
			}
			if (inn_peak_vals[c] <= outMat.at<uint8_t>(r, c)) {
				inn_peak_vals[c] = outMat.at<uint8_t>(r, c);
				inn_peak_locs[c] = r;
			}
		}
	}

	int out_peak_mean = 0;
	int out_peak_thresh = 0;
	for (int c = 0; c < size; c++) {
		out_peak_mean += out_peak_sums[c];
	}
	out_peak_mean /= size;
	out_peak_thresh = (int)(out_peak_mean * 0.60f);
	
	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	if (isDisc)
	{
		int thresh, threshMax, threshMin;
		threshMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * 4.0f);
		threshMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * 1.0f);

		// Optic disc area enclosing its raised nerves. 
		int dpos1 = -1, dpos2 = -1, epos1 = -1, epos2 = -1, dcent = -1;
		bool found = false;

		int disc_w = disc2 - disc1 + 1;
		if (disc_w > 1) {
			int disc_r = max(disc_w, 24) / 2;
			dpos1 = max(disc1 - disc_r, 0);
			dpos2 = min(disc2 + disc_r, size - 1);
			dcent = (disc2 - disc1) / 2;
			epos1 = max(disc1 - disc_r * 2, 0);
			epos2 = min(disc2 + disc_r * 2, size - 1);
			found = true;
		}

		//LogD() << dpos1 << ", " << dpos2 ;
		//LogD() << epos1  << ", " << epos2 ;

		int moves, range;
		// retry = false;

		if (found) {
			for (int i = 0; i < size; i++) {
				if ((i >= dpos1 && i <= dpos2)) {
					range = (int)((outer[i] - inner[i])*(retry ? 0.35f : 0.45f));
					upper[i] = max(outer[i] - range, inner[i]);
					range = min((int)((outer[i] - inner[i])*(retry ? 0.05f : 0.10f)), PATH_NFL_LOWER_SPAN_DISC);
					lower[i] = max(outer[i] - range, upper[i]);
				}
				else if ((i >= epos1 && i <= epos2)) {
					range = (int)((outer[i] - inner[i])*(retry ? 0.75f : 0.65f));
					upper[i] = max(outer[i] - range, inner[i]);
					range = min((int)((outer[i] - inner[i])*(retry ? 0.15f : 0.15f)), PATH_NFL_LOWER_SPAN_DISC);
					lower[i] = max(outer[i] - range, upper[i]);
				}
				else {
					range = (int)((outer[i] - inner[i])*(retry ? 0.85f : 0.85f));
					upper[i] = max(outer[i] - range, inner[i]);
					range = max((int)((outer[i] - inner[i])*(retry ? 0.25f : 0.25f)), PATH_NFL_LOWER_SPAN_DISC);
					lower[i] = max(outer[i] - range, upper[i]);
				}
				moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i] + 1) : moves);
				delta[i] = min(max(moves, movesMin), movesMax);
			}
		}
		else {
			for (int i = 0; i < size; i++) {
				range = (int)((outer[i] - inner[i])*0.85f);
				upper[i] = max(outer[i] - range, inner[i]);
				range = max((int)((outer[i] - inner[i])*0.05f), PATH_NFL_LOWER_SPAN_DISC);
				lower[i] = max(outer[i] - range, upper[i]);
				moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i] + 1) : moves);
				delta[i] = min(max(moves, movesMin), movesMax);
			}
		}

		for (int c = 0, r = 0; c < size; c++) {
			if ((c >= dpos1 && c <= dpos2)) {
				if (retry) {
					thresh = max(inn_peak_vals[c], out_peak_vals[c]);
					thresh = (int)(thresh * 0.75f);
				}
				else {
					thresh = max(inn_peak_vals[c], out_peak_vals[c]);
					thresh = (int)(thresh * 0.75f);
				}
				thresh = min(max(thresh, threshMin), threshMax);
			}
			else if ((c >= epos1 && c <= epos2)) {
				thresh = max(inn_peak_vals[c], out_peak_vals[c]);
				thresh = (int)(thresh * 0.75f);
				thresh = min(max(thresh, threshMin), threshMax);
			}
			else {
				thresh = max(inn_peak_vals[c], out_peak_vals[c]);
				thresh = (int)(thresh * 0.95f);
				thresh = min(max(thresh, threshMin), threshMax);
			}

			int start = upper[c];
			int close = lower[c];
			int count = -1, max_idx = -1, max_val = 0;
			for (r = start, count = 0; r <= close; r++, count++) {
				if (outMat.at<uint8_t>(r, c) >= thresh) {
					max_val = outMat.at<uint8_t>(r, c);
					max_idx = r;
				}
			}

			if ((c >= dpos1 && c <= dpos2)) {
				if (out_peak_vals[c] >= out_peak_thresh) {
					if (!found) {
						upper[c] = max(max_idx, upper[c]);
					}
				}
				else {
					upper[c] = (c > 0 ? max(upper[c - 1], upper[c]) : upper[c]);
				}
			}
			else if ((c >= epos1 && c <= epos2)) {
				if (out_peak_vals[c] >= out_peak_thresh) {
					if (!found) {
						upper[c] = max(max_idx, upper[c]);
					}
				}
				else {
					upper[c] = (c > 0 ? max(upper[c - 1], upper[c]) : upper[c]);
				}
			}
			else {
				if (inn_peak_vals[c] < thresh) {
					// if (found) {
						upper[c] = inner[c];
						lower[c] = min(upper[c] + 5, outer[c]);
					// }
				}
				else {
					if (out_peak_vals[c] >= out_peak_thresh) {
						if (!found) {
							upper[c] = max(max_idx, upper[c]);
						}
						// lower[c] = inner[c]; min(upper[c] + 5, outer[c]);
					}
					else {
						upper[c] = (c > 0 ? max(upper[c - 1], upper[c]) : upper[c]);
					}
				}
			}
		}
	}
	else {
		int moves, range;
		for (int i = 0; i < size; i++) {
			range = min((int)((outer[i] - inner[i])*0.95f), PATH_NFL_UPPER_SPAN);
			upper[i] = max(outer[i] - range, inner[i]);
			range = max((int)((outer[i] - inner[i])*0.25f), PATH_NFL_LOWER_SPAN);
			lower[i] = max(outer[i] - range, upper[i]);
			moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i] + 1) : moves);
			delta[i] = min(max(moves, movesMin), movesMax);
		}

		int nfl_idx;
		int thresh, threshMax, threshMin;
		threshMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * 3.0f);
		threshMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * 1.0f);

		for (int c = 0, r = 0; c < size; c++) {
			thresh = (int)(out_peak_vals[c] * 0.90f);
			thresh = min(max(thresh, threshMin), threshMax);

			// Initial region between ILM and IPL.
			int start = upper[c];
			int close = lower[c]; // -(int)(((outer[c] - inner[c]) * 0.15f));
			int max_idx, max_val = 0;

			int count = -1; max_idx = -1; max_val = 0;
			for (r = start, count = 0; r <= close; r++, count++) {
				if (outMat.at<uint8_t>(r, c) >= max_val) {
					max_val = outMat.at<uint8_t>(r, c);
					max_idx = r;
				}
			}

			if (max_val >= thresh) {
				nfl_idx = max_idx;
				upper[c] = max(nfl_idx - 3, inner[c]);
				lower[c] = max(close, upper[c]);
			}
			else {
				upper[c] = max(start, inner[c]);
				lower[c] = min(start + 5, lower[c]);
			}
		}
	}

	return true;
}


bool RetSegm::OptimalPath::designPathConstraintsOfNFL4(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIPL, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, float rangeX, bool isDisc, int disc1, int disc2, bool retry)
{
	auto inner = layerILM->getYs();
	auto outer = layerIPL->getYs();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();
	int pixelsPerMM = (int)(size / rangeX);

	if (size <= 0 || height <= 0) {
		return false;
	}

	SegmImage imgOut;
	Coarse::createAverageMap(imgSrc, &imgOut, 5, 5);
	Mat outMat = imgOut.getCvMatConst();

	// Peak values around ILM and RPE layer. 
	auto inn_peak_vals = vector<int>(size, 0);
	auto out_peak_vals = vector<int>(size, 0);
	auto inn_peak_tops = vector<int>(size, 0);
	auto out_peak_tops = vector<int>(size, 0);
	auto inn_peak_locs = upper;
	auto out_peak_locs = lower;

	int start, close;
	int gsum = 0, gcnt = 0;
	for (int c = 0, r = 0; c < size; c++) {
		start = upper[c];
		close = lower[c];
		gsum = 0, gcnt = 0;
		for (r = start; r < close; r++) {
			if (r < 0 || r >= height) {
				continue;
			}
			if (out_peak_tops[c] <= outMat.at<uint8_t>(r, c)) {
				out_peak_tops[c] = outMat.at<uint8_t>(r, c);
				out_peak_locs[c] = r;
			}
			gsum += outMat.at<uint8_t>(r, c);
			gcnt++;
		}
		if (gcnt > 0) {
			out_peak_vals[c] = gsum / gcnt;
		}

		start = inner[c];
		close = inner[c] + (outer[c] - inner[c]);
		gsum = gcnt = 0;
		for (r = start; r < close; r++) {
			if (r < 0 || r >= height) {
				continue;
			}
			gsum += outMat.at<uint8_t>(r, c);
			gcnt++;
		}
		if (gcnt > 0) {
			inn_peak_vals[c] = gsum / gcnt;
		}
	}

	int out_peak_mean = 0;
	int out_tops_mean = 0;
	int out_peak_thresh = 0;
	int out_tops_thresh = 0;
	for (int i = 0; i < size; i++) {
		out_peak_mean += out_peak_vals[i];
		out_tops_mean += out_peak_tops[i];
	}
	out_peak_mean = out_peak_mean / size;
	out_tops_mean = out_tops_mean / size;
	out_peak_thresh = (int)(out_peak_mean * 0.75f);
	out_tops_thresh = (int)(out_tops_mean * 0.75f);

	/*
	for (int i = 0; i < size; i++) {
		LogD() << i << ", " << inn_peak_vals[i] << ", " << out_peak_vals[i] << " => " << out_peak_vals[i] * 0.70f << " : " << out_peak_tops[i] << " => " << out_peak_tops[i] * 0.70f << " : " << out_peak_mean;
	}
	*/

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	const int movesMin = (isDisc ? PATH_NFL_MOVES_MIN_DISC : PATH_NFL_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_NFL_MOVES_MAX_DISC : PATH_NFL_MOVES_MAX);

	if (isDisc)
	{
		int thresh, threshMax, threshMin;
		threshMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * 4.0f);
		threshMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * 1.0f);

		// Optic disc area enclosing its raised nerves. 
		int dpos1 = -1, dpos2 = -1, epos1 = -1, epos2 = -1;
		bool found = false;

		int disc_w = disc2 - disc1 + 1;
		if (disc_w > 1) {
			int disc_r = max(disc_w, 24) / 2;
			dpos1 = max(disc1 - disc_r, 0);
			dpos2 = min(disc2 + disc_r, size - 1);
			epos1 = max(disc1 - disc_r * 2, 0);
			epos2 = min(disc2 + disc_r * 2, size - 1);
			found = true;
		}

		int moves, range;
		// retry = false;

		for (int c = 0; c < size; c++) {
			thresh = max(out_peak_tops[c], out_peak_mean);
			thresh = int(thresh * 0.70f);
			thresh = min(max(thresh, threshMin), threshMax);

			if (c >= epos1 && c <= epos2) {
				if (c >= disc1 && c <= disc2) {
					range = (int)((outer[c] - inner[c]) * 0.25f);
					upper[c] = max(outer[c] - range, inner[c]);
					range = min((int)((outer[c] - inner[c]) * 0.15f), PATH_NFL_LOWER_SPAN_DISC);
					lower[c] = max(min(lower[c], outer[c] - range), upper[c]);
				}
				else {
					thresh = max(out_peak_tops[c], out_peak_mean);
					thresh = int(thresh * 0.85f);
					thresh = min(max(thresh, threshMin), threshMax);

					if (c >= dpos1 && c <= dpos2) {
						if (inn_peak_vals[c] >= thresh) {
							range = min((int)((outer[c] - inner[c]) * 0.35f), 9);
						}
						else {
							range = min((int)((outer[c] - inner[c]) * 0.55f), 17);
						}
					}
					else {
						if (inn_peak_vals[c] >= thresh) {
							range = min((int)((outer[c] - inner[c]) * 0.45f), 9);
						}
						else {
							range = min((int)((outer[c] - inner[c]) * 0.65f), 17);
						}
					}
					upper[c] = max(outer[c] - range, inner[c]);
					range = min((int)((outer[c] - inner[c]) * 0.15f), PATH_NFL_LOWER_SPAN_DISC);
					lower[c] = max(min(lower[c], outer[c] - range), upper[c]);
				}
			}
			else {
				if (inn_peak_vals[c] < thresh) {
					upper[c] = inner[c];
					range = min((int)((outer[c] - inner[c]) * 0.50f), 9);
					lower[c] = min(inner[c] + range, outer[c]);
					range = min((int)((outer[c] - inner[c]) * 0.15f), PATH_NFL_LOWER_SPAN_DISC);
					lower[c] = max(min(lower[c], outer[c] - range), upper[c]);
				}
				else {
					thresh = max(out_peak_tops[c], out_peak_mean);
					thresh = int(thresh * 0.85f);
					thresh = min(max(thresh, threshMin), threshMax);

					if (!found && inn_peak_vals[c] >= thresh) {
						range = min((int)((outer[c] - inner[c]) * 0.55f), 13);
						upper[c] = max(outer[c] - range, inner[c]);
						range = min((int)((outer[c] - inner[c]) * 0.15f), PATH_NFL_LOWER_SPAN_DISC);
						lower[c] = max(outer[c] - range, upper[c]);
					}
					else {
						range = min((int)((outer[c] - inner[c]) * 0.75f), 25);
						upper[c] = max(outer[c] - range, inner[c]);
						range = min((int)((outer[c] - inner[c]) * 0.15f), PATH_NFL_LOWER_SPAN_DISC);
						lower[c] = max(outer[c] - range, upper[c]);

						if (out_peak_vals[c] < out_peak_thresh) {
							upper[c] = (c > 0 ? max(upper[c - 1], upper[c]) : upper[c]);
							lower[c] = (c > 0 ? max(lower[c - 1], lower[c]) : lower[c]);
						}
					}
				}
			}

			if (found) {
				int discRange = (int)(4.0f * pixelsPerMM);
				if ((c < disc1 && abs(c - disc1) > discRange) ||
					(c > disc2 && abs(c - disc2) > discRange)) {
					range = max((int)((outer[c] - inner[c])*0.99f), PATH_NFL_UPPER_SPAN_DISC);
					upper[c] = max(outer[c] - range, inner[c]);
					range = max((int)((outer[c] - inner[c])*0.65f), PATH_NFL_LOWER_SPAN_DISC);
					lower[c] = max(outer[c] - range, upper[c]);
					moves = (c < (size - 1) ? abs(inner[c + 1] - inner[c] + 1) : moves);
					delta[c] = min(max(moves, movesMin), movesMax);
				}
			}

			moves = (c < (size - 1) ? abs(inner[c + 1] - inner[c] + 1) : moves);
			delta[c] = min(max(moves, movesMin), movesMax);
		}

		for (int c = 1; c < size; c++) {
			if (upper[c] > lower[c - 1]) {
				upper[c] = lower[c - 1];
			}
			if (lower[c] < upper[c - 1]) {
				lower[c] = upper[c - 1];
			}
		}

		/*
		int thresh, threshMax, threshMin;
		threshMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * 4.0f);
		threshMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * 1.0f);

		// Optic disc area enclosing its raised nerves. 
		int dpos1 = -1, dpos2 = -1, epos1 = -1, epos2 = -1;
		bool found = false;

		int disc_w = disc2 - disc1 + 1;
		if (disc_w > 1) {
			int disc_r = max(disc_w, 24) / 2;
			dpos1 = max(disc1 - disc_r, 0);
			dpos2 = min(disc2 + disc_r, size - 1);
			epos1 = max(disc1 - disc_r * 2, 0);
			epos2 = min(disc2 + disc_r * 2, size - 1);
			found = true;
		}

		//LogD() << dpos1 << ", " << dpos2 ;
		//LogD() << epos1  << ", " << epos2 ;

		int moves, range;
		// retry = false;

		for (int c = 0; c < size; c++) {
			thresh = max(out_peak_tops[c], out_peak_mean);
			thresh = int(thresh * 0.70f);
			thresh = min(max(thresh, threshMin), threshMax);

			if (c >= epos1 && c <= epos2) {
				if (c >= disc1 && c <= disc2) {
					range = (int)((outer[c] - inner[c]) * 0.25f);
					upper[c] = max(outer[c] - range, inner[c]);
					range = min((int)((outer[c] - inner[c]) * 0.15f), PATH_NFL_LOWER_SPAN_DISC);
					lower[c] = max(min(lower[c], outer[c] - range), upper[c]);
				}
				else {
					thresh = max(out_peak_tops[c], out_peak_mean);
					thresh = int(thresh * 0.85f);
					thresh = min(max(thresh, threshMin), threshMax);

					if (c >= dpos1 && c <= dpos2) {
						if (inn_peak_vals[c] >= thresh) {
							range = min((int)((outer[c] - inner[c]) * 0.35f), 9);
						}
						else {
							range = min((int)((outer[c] - inner[c]) * 0.55f), 17);
						}
					}
					else {
						if (inn_peak_vals[c] >= thresh) {
							range = min((int)((outer[c] - inner[c]) * 0.45f), 9);
						}
						else {
							range = min((int)((outer[c] - inner[c]) * 0.65f), 17);
						}
					}
					upper[c] = max(outer[c] - range, inner[c]);
					range = min((int)((outer[c] - inner[c]) * 0.15f), PATH_NFL_LOWER_SPAN_DISC);
					lower[c] = max(min(lower[c], outer[c] - range), upper[c]);
				}
			}
			else {
				if (inn_peak_vals[c] < thresh) {
					upper[c] = inner[c];
					range = min((int)((outer[c] - inner[c]) * 0.50f), 9);
					lower[c] = min(inner[c] + range, outer[c]);
					range = min((int)((outer[c] - inner[c]) * 0.15f), PATH_NFL_LOWER_SPAN_DISC);
					lower[c] = max(min(lower[c], outer[c] - range), upper[c]);
				}
				else {
					thresh = max(out_peak_tops[c], out_peak_mean);
					thresh = int(thresh * 0.85f);
					thresh = min(max(thresh, threshMin), threshMax);

					if (!found && inn_peak_vals[c] >= thresh) {
						range = min((int)((outer[c] - inner[c]) * 0.55f), 13);
						upper[c] = max(outer[c] - range, inner[c]);
						range = min((int)((outer[c] - inner[c]) * 0.15f), PATH_NFL_LOWER_SPAN_DISC);
						lower[c] = max(outer[c] - range, upper[c]);
					}
					else {
						range = min((int)((outer[c] - inner[c]) * 0.75f), 25);
						upper[c] = max(outer[c] - range, inner[c]);
						range = min((int)((outer[c] - inner[c]) * 0.15f), PATH_NFL_LOWER_SPAN_DISC);
						lower[c] = max(outer[c] - range, upper[c]);

						if (out_peak_vals[c] < out_peak_thresh) {
							upper[c] = (c > 0 ? max(upper[c - 1], upper[c]) : upper[c]);
							lower[c] = (c > 0 ? max(lower[c - 1], lower[c]) : lower[c]);
						}
					}
				}
			}

			moves = (c < (size - 1) ? abs(inner[c + 1] - inner[c] + 1) : moves);
			delta[c] = min(max(moves, movesMin), movesMax);
		}

		for (int c = 1; c < size; c++) {
			if (upper[c] > lower[c - 1]) {
				upper[c] = lower[c - 1];
			}
			if (lower[c] < upper[c - 1]) {
				lower[c] = upper[c - 1];
			}
		}
		*/
	}
	else {
		int moves, range;
		for (int i = 0; i < size; i++) {
			range = min((int)((outer[i] - inner[i])*0.95f), PATH_NFL_UPPER_SPAN);
			upper[i] = max(outer[i] - range, inner[i]);
			range = max((int)((outer[i] - inner[i])*0.25f), PATH_NFL_LOWER_SPAN);
			lower[i] = max(outer[i] - range, upper[i]);
			moves = (i < (size - 1) ? abs(inner[i + 1] - inner[i] + 1) : moves);
			delta[i] = min(max(moves, movesMin), movesMax);
		}

		int nfl_idx;
		int thresh, threshMax, threshMin;
		threshMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * 3.0f);
		threshMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * 1.0f);

		for (int c = 0, r = 0; c < size; c++) {
			thresh = (int)(out_peak_vals[c] * 0.90f);
			thresh = min(max(thresh, threshMin), threshMax);

			// Initial region between ILM and IPL.
			int start = upper[c];
			int close = lower[c]; // -(int)(((outer[c] - inner[c]) * 0.15f));
			int max_idx, max_val = 0;

			int count = -1; max_idx = -1; max_val = 0;
			for (r = start, count = 0; r <= close; r++, count++) {
				if (outMat.at<uint8_t>(r, c) >= max_val) {
					max_val = outMat.at<uint8_t>(r, c);
					max_idx = r;
				}
			}

			if (max_val >= thresh) {
				nfl_idx = max_idx;
				upper[c] = max(nfl_idx - 3, inner[c]);
				lower[c] = max(close, upper[c]);
			}
			else {
				upper[c] = max(start, inner[c]);
				lower[c] = min(start + 5, lower[c]);
			}
		}
	}

	return true;
}


bool RetSegm::OptimalPath::designPathConstraintsOfNFL5(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIPL, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, float rangeX, bool isDisc, int disc1, int disc2, bool retry)
{
	auto inner = layerILM->getYs();
	auto outer = layerIPL->getYs();

	int size = (int)inner.size();
	int height = imgSrc->getHeight();
	int pixelsPerMM = (int)(size / rangeX);

	if (size <= 0 || height <= 0) {
		return false;
	}

	SegmImage imgOut;
	Coarse::createAverageMap(imgSrc, &imgOut, 5, 3);
	Mat outMat = imgOut.getCvMatConst();

	auto peak_vals = vector<int>(size, -1);
	auto peak_locs = inner;

	for (int c = 0, r = 0; c < size; c++) {
		for (int r = inner[c]; r < outer[c]; r++) {
			if (peak_vals[c] < outMat.at<uint8_t>(r, c)) {
				peak_vals[c] = outMat.at<uint8_t>(r, c);
				peak_locs[c] = r;
			}
		}
	}

	int thread = (int)(imgSrc->getMean() + imgSrc->getStddev() * 4.0f);
	thread = min(max(thread, 90), 190);

	const int movesMin = (isDisc ? PATH_NFL_MOVES_MIN_DISC : PATH_NFL_MOVES_MIN);
	const int movesMax = (isDisc ? PATH_NFL_MOVES_MAX_DISC : PATH_NFL_MOVES_MAX);
	
	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	int moves;
	for (int c = 0, r = 0; c < size; c++) {
		if (peak_vals[c] < thread) {
			upper[c] = inner[c];
			lower[c] = min(inner[c] + 5, outer[c]);
		}
		else {
			upper[c] = peak_locs[c];
			lower[c] = outer[c];
		}
		upper[c] = max(upper[c], 0);
		lower[c] = min(lower[c], height - 1);

		if (c < size - 1) {
			moves = abs(outer[c + 1] - outer[c]);
		}
		delta[c] = min(max(moves, movesMin), movesMax);
	}
	return true;
}


bool RetSegm::OptimalPath::createPathCostMapOfILM(const SegmImage * imgSrc, SegmImage * imgCost,
									std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
									bool isDisc)
{
	Mat srcMat, mask;
	imgSrc->getCvMat().copyTo(srcMat);

	float ratioMin = (isDisc ? PATH_ILM_STDDEV_TO_GRAY_MIN_DISC : PATH_ILM_STDDEV_TO_GRAY_MIN);
	float ratioMax = (isDisc ? PATH_ILM_STDDEV_TO_GRAY_MAX_DISC : PATH_ILM_STDDEV_TO_GRAY_MAX);
	int grayMin = (int)(imgSrc->getMean() + imgSrc->getStddev()*ratioMin);
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*ratioMax);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	Mat kernel = Mat::zeros(PATH_ILM_KERNEL_ROWS, PATH_ILM_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows/2; r++) {
		for(int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = (kernel.cols/3); c < (2*kernel.cols)/3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	/*
	Mat kernel = Mat::zeros(7, 3, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	*/

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Leave only positive gradients, and normalize in range between [0, 255].
	// Mat mask = outMat < 0;
	// outMat.setTo(0.0f, mask);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c-1] && k <= lower[c-1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
			//line += to_string(outMat.at<int>(r, c));
			//line += " ";
		}
		//DebugOut() << line;
	}
	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createPathCostMapOfOut(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, mask;
	imgSrc->getCvMat().copyTo(srcMat);

	Mat kernel = Mat::zeros(PATH_IOS_KERNEL_ROWS, PATH_IOS_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;
	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createPathCostMapOfIOS(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, mask;
	imgSrc->getCvMat().copyTo(srcMat);

	/*
	int grayMin = 0; // (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*PATH_IOS_STDDEV_TO_GRAY_MAX);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	Mat kernel = Mat::zeros(PATH_IOS_KERNEL_ROWS, PATH_IOS_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols/3; c < (2*kernel.cols)/3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}
	*/

	Mat kernel = Mat::zeros(PATH_IOS_KERNEL_ROWS, PATH_IOS_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;
	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createPathCostMapOfRPE(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, mask;
	imgSrc->getCvMat().copyTo(srcMat);

	/*
	int grayMin = (int)(imgSrc->getMean() + imgSrc->getStddev());
	int grayMax = 255; // (int)(imgSrc->getMean() + imgSrc->getStddev()*PATH_IOS_STDDEV_TO_GRAY_MAX);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	Mat kernel = Mat::zeros(PATH_OPR_KERNEL_ROWS, PATH_OPR_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}
	*/

	Mat kernel = Mat::zeros(PATH_RPE_KERNEL_ROWS, PATH_RPE_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;
	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createPathCostMapOfOPR(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, mask;
	imgSrc->getCvMat().copyTo(srcMat);

	
	int grayMin = 0;// (int)(imgSrc->getMean() + imgSrc->getStddev()*0.0f);
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*3.0f);

	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);
	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);

	Mat kernel = Mat::zeros(PATH_OPR_KERNEL_ROWS, PATH_OPR_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	/*
	Mat kernel = Mat::zeros(PATH_OPR_KERNEL_ROWS, PATH_OPR_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	*/

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;
	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c-1] && k <= lower[c-1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createPathCostMapOfBRM(const SegmImage * imgSrc, SegmImage * imgCost,
						std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, mask;
	imgSrc->getCvMat().copyTo(srcMat);

	int grayMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * 1.0f);
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * 6.0f);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	Mat kernel = Mat::zeros(PATH_BRM_KERNEL_ROWS, PATH_BRM_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	/*
	Mat kernel = Mat::zeros(5, 3, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	*/

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;
	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createPathCostMapOfOPL(const SegmImage * imgSrc, SegmImage * imgCost, 
			std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
			bool isDisc, int disc1, int disc2)
{
	Mat srcMat, mask;
	imgSrc->getCvMat().copyTo(srcMat);

	float ratioMin = (isDisc ? PATH_OPL_STDDEV_TO_GRAY_MIN_DISC : PATH_OPL_STDDEV_TO_GRAY_MIN);
	float ratioMax = (isDisc ? PATH_OPL_STDDEV_TO_GRAY_MAX_DISC : PATH_OPL_STDDEV_TO_GRAY_MAX);
	int grayMin = 0; // (int)(imgSrc->getMean() + imgSrc->getStddev() * ratioMin);
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * ratioMax);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	Mat kernel;

	bool found = (isDisc && (disc2 - disc1) > 0);
	if (found) {
		kernel = Mat::zeros(5, 3, CV_32F);
	}
	else {
		kernel = Mat::zeros(5, 3, CV_32F);
	}

	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	/*
	float value, minPeak;
	for (int c = 0, r = 0; c < outMat.cols; c++) {
		minPeak = 0.0f;
		for (r = lower[c]; r >= upper[c]; r--) {
			value = outMat.at<float>(r, c);
			if (value <= minPeak) {
				minPeak = value;
			}
			else {
				if (minPeak < 0.0f) {
					break;
				}
			}
		}

		for (; r >= upper[c]; r--) {
			outMat.at<float>(r, c) = 0.0f;
		}
	}
	*/

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;
	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createPathCostMapOfIPL(const SegmImage * imgSrc, SegmImage * imgCost, 
			std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
			bool isDisc, int disc1, int disc2)
{
	Mat srcMat;
	imgSrc->getCvMat().copyTo(srcMat);

	float ratioMin = (isDisc ? PATH_IPL_STDDEV_TO_GRAY_MIN_DISC : PATH_IPL_STDDEV_TO_GRAY_MIN);
	float ratioMax = (isDisc ? PATH_IPL_STDDEV_TO_GRAY_MAX_DISC : PATH_IPL_STDDEV_TO_GRAY_MAX);
	int grayMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * ratioMin);
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * ratioMax);

	Mat kernel, mask;
	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	bool found = (isDisc && (disc2 - disc1) > 0);
	if (found) {
		kernel = Mat::zeros(PATH_IPL_KERNEL_ROWS_DISC, PATH_IPL_KERNEL_COLS_DISC, CV_32F);
	}
	else {
		kernel = Mat::zeros(PATH_IPL_KERNEL_ROWS, PATH_IPL_KERNEL_COLS, CV_32F);
	}

	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	/*
	float value, minPeak;
	for (int c = 0, r = 0; c < outMat.cols; c++) {
		minPeak = 0.0f;
		for (r = upper[c]; r <= lower[c]; r++) {
			value = outMat.at<float>(r, c);
			if (value <= minPeak) {
				minPeak = value;
			}
			else {
				if (minPeak < 0.0f) {
					break;
				}
			}
		}

		for (; r <= lower[c]; r++) {
			outMat.at<float>(r, c) = 0.0f;
		}
	}
	*/

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;
	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::createPathCostMapOfNFL(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, 
													bool isDisc, int disc1, int disc2)
{
	Mat srcMat;
	imgSrc->getCvMat().copyTo(srcMat);

	float ratioMin = (isDisc ? PATH_NFL_STDDEV_TO_GRAY_MIN_DISC : PATH_NFL_STDDEV_TO_GRAY_MIN);
	// float ratioMax = (isDisc ? PATH_NFL_STDDEV_TO_GRAY_MAX_DISC : PATH_NFL_STDDEV_TO_GRAY_MAX);
	int grayMin = (int)(imgSrc->getMean() + imgSrc->getStddev() * ratioMin);
	// int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev() * ratioMax);

	Mat kernel, mask;
	// mask = srcMat > grayMax;
	// srcMat.setTo(grayMax, mask);
	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	bool found = (isDisc && (disc2 - disc1) > 0);
	if (found) {
		kernel = Mat::zeros(PATH_NFL_KERNEL_ROWS_DISC, PATH_NFL_KERNEL_COLS_DISC, CV_32F);
	}
	else {
		kernel = Mat::zeros(PATH_NFL_KERNEL_ROWS, PATH_NFL_KERNEL_COLS, CV_32F);
	}

	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE); // BORDER_REFLECT);

	/*
	float d_rate, g_rate;
	for (int c = 0, r = 0; c < outMat.cols; c++) {
		for (r = upper[c]; r <= lower[c]; r++) {
			if (outMat.at<float>(r, c) < 0.0f) {
				d_rate = (1.0f - (float)(r - upper[c]) / (float)(lower[c] - upper[c] + 1));
				g_rate = (float)srcMat.at<uint8_t>(r, c) / 255.0f;
				outMat.at<float>(r, c) *= (g_rate);
			}
		}
	}
	*/

	for (int c = 0, r = 0; c < outMat.cols; c++) {
		int dist = 0;
		for (r = upper[c]; r <= lower[c]; r++, dist++) {
			outMat.at<float>(r, c) += 0.001f * dist;
		}
	}

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;
	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				// if (k >= 0 && k < outMat.rows) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::searchPathWithMinCost(const SegmImage * imgCost, 
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												std::vector<int>& output)
{
	const Mat costMat = imgCost->getCvMatConst();
	auto minPath = vector<int>(costMat.cols, -1);

	// Starting from the right most column. 
	int lastIdx = -1, nextIdx = -1;
	float minCost = PATH_COST_MAX;
	int r, c;

	c = costMat.cols - 1;
	for (r = upper[c]; r <= lower[c]; r++) {
		if (costMat.at<float>(r, c) <= minCost) {
			minCost = costMat.at<float>(r, c);
			lastIdx = r;
		}
	}
	minPath[c] = (lastIdx < 0 ? ((upper[c] + lower[c])/2) : lastIdx);
	lastIdx = minPath[c];

	// Follow the minimum cost path in reverse order. 
	// Next point should be within the allowed vertical distance. 
	int rBeg, rEnd;
	for (c = costMat.cols - 2; c >= 0; c--) {
		rBeg = max(lastIdx - delta[c], upper[c]);
		rEnd = min(lastIdx + delta[c], lower[c]);

		if (rBeg > lower[c] || rEnd < upper[c]) {
			minPath[c] = (rBeg > lower[c] ? lower[c] : upper[c]);
		}
		else {
			// Note that if any element with cost calculated from intensities (less than maximum at default) 
			// is not found at this column, the last index would go down toward the bottom of map. 
			minCost = PATH_COST_MAX;
			nextIdx = -1;
			for (r = rBeg; r <= rEnd; r++) {
				if (costMat.at<float>(r, c) <= minCost) {
					minCost = costMat.at<float>(r, c);
					nextIdx = r;
				}
			}
			minPath[c] = (nextIdx < 0 ? ((upper[c] + lower[c]) / 2) : nextIdx);
		}
		lastIdx = minPath[c];
	}

	output = minPath;
	return true;
}


bool RetSegm::OptimalPath::findBoundaryToILM(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut, SegmImage * imgCost, SegmLayer * layerILM)
{
	const Mat srcMat = imgSrc->getCvMatConst();
	layerInn->resize(imgSrc->getWidth(), imgSrc->getHeight());
	layerOut->resize(imgSrc->getWidth(), imgSrc->getHeight());

	vector<int> upperEnds, lowerEnds, moveDelta;

	float sampleRatio = imgSrc->getSampleRatioHeight();
	int upperRange = (int)(PATH_ILM_UPPER_RANGE*sampleRatio);
	int lowerRange = (int)(PATH_ILM_LOWER_RANGE*sampleRatio);
	int moveSpan = (int)(PATH_ILM_CHANGE_MAX*sampleRatio);
	int edgeSize = (int)(PATH_ILM_EDGE_SIZE*sampleRatio);

	imposePathConstraintsForILM(imgSrc, layerInn, layerOut, upperEnds, lowerEnds, moveDelta, upperRange, lowerRange, moveSpan);
	layerILM->getDeltas() = moveDelta;

	createPathCostMap(imgSrc, upperEnds, lowerEnds, moveDelta, PATH_ILM_THRESH1, PATH_ILM_THRESH2, edgeSize, true, imgCost);
	searchPathMinCost(imgCost, upperEnds, lowerEnds, moveDelta, layerILM);

	sampleRatio = imgSrc->getSampleRatioWidth();
	int smoothWidth = (int)(PATH_ILM_SMOOTH_WIDTH*sampleRatio);
	int smoothDegree = PATH_ILM_SMOOTH_DEGREE;
	auto outs = CppUtil::SgFilter::smoothInts(layerILM->getYs(), smoothWidth, smoothDegree);
	layerILM->initialize(outs, imgSrc->getWidth(), imgSrc->getHeight());

	return true;
}


bool RetSegm::OptimalPath::findBoundaryToIOS(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut, SegmImage * imgCost, SegmLayer * layerIOS)
{
	const Mat srcMat = imgSrc->getCvMatConst();

	vector<int> upperEnds, lowerEnds, moveDelta;

	float sampleRatio = imgSrc->getSampleRatioHeight();
	int upperRange = (int)(PATH_IOS_UPPER_RANGE*sampleRatio);
	int lowerRange = (int)(PATH_IOS_LOWER_RANGE*sampleRatio);
	int moveSpan = (int)(PATH_IOS_CHANGE_MAX*sampleRatio);
	int edgeSize = (int)(PATH_IOS_EDGE_SIZE*sampleRatio);

	imposePathConstraintsForIOS(imgSrc, layerInn, layerOut, upperEnds, lowerEnds, moveDelta, upperRange, lowerRange, moveSpan);

	createPathCostMap(imgSrc, upperEnds, lowerEnds, moveDelta, PATH_IOS_THRESH1, PATH_IOS_THRESH2, edgeSize, true, imgCost);
	searchPathMinCost(imgCost, upperEnds, lowerEnds, moveDelta, layerIOS);

	sampleRatio = imgSrc->getSampleRatioWidth();
	int smoothWidth = (int)(PATH_IOS_SMOOTH_WIDTH*sampleRatio);
	int smoothDegree = PATH_IOS_SMOOTH_DEGREE;
	auto outs = CppUtil::SgFilter::smoothInts(layerIOS->getYs(), smoothWidth, smoothDegree);
	layerIOS->initialize(outs, imgSrc->getWidth(), imgSrc->getHeight());

	return true;
}


bool RetSegm::OptimalPath::imposePathConstraintsForILM(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, int upperRange, int lowerRange, int moveSpan)
{
	auto inner = layerInn->getPoints();
	auto outer = layerOut->getPoints();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	auto upperLimit = vector<int>(size, 0);
	auto lowerLimit = vector<int>(size, height - 1);

	// Limit lower range to between inner and outer boundary. 
	for (int i = 0; i < size; i++) {
		if (inner[i].valid && outer[i].valid) {
			lowerLimit[i] = inner[i].y + (outer[i].y - inner[i].y) / 3;
		}
	}

	int last = -1;
	int dist, yval;
	float rate;
	for (int i = 0; i < size; i++) {
		if (inner[i].valid) {
			dist = i - last;
			if (dist > 1) {
				if (last >= 0) {
					rate = (float)(inner[i].y - inner[last].y) / dist;
					for (int j = last + 1, k = 1; j < i; j++, k++) {
						yval = (int)(inner[last].y + rate*k);
						upper[j] = yval - upperRange;
						lower[j] = yval + upperRange;
						delta[j] = (int)rate + 1;
						upper[j] = (upper[j] < 0 ? 0 : upper[j]);
						lower[j] = (lower[j] > lowerLimit[j] ? lowerLimit[j] : lower[j]);
						delta[j] = (delta[j] < moveSpan ? moveSpan : delta[j]);
					}
				}
				else {
					yval = inner[i].y;
					for (int j = 0; j < i; j++) {
						upper[j] = yval - upperRange;
						lower[j] = yval + upperRange;
						delta[j] = moveSpan;
						upper[j] = (upper[j] < 0 ? 0 : upper[j]);
						lower[j] = (lower[j] > lowerLimit[j] ? lowerLimit[j] : lower[j]);
					}
				}
			}

			yval = inner[i].y;
			upper[i] = yval - upperRange;
			lower[i] = yval + lowerRange;
			delta[i] = moveSpan;
			upper[i] = (upper[i] < 0 ? 0 : upper[i]);
			lower[i] = (lower[i] > lowerLimit[i] ? lowerLimit[i] : lower[i]);
			last = i;
		}
	}

	if (inner[size - 1].valid == false) {
		if (last >= 0) {
			yval = (int)inner[last].y;
			for (int j = last + 1; j < size; j++) {
				upper[j] = yval - upperRange;
				lower[j] = yval + upperRange;
				delta[j] = moveSpan;
				upper[j] = (upper[j] < 0 ? 0 : upper[j]);
				lower[j] = (lower[j] > lowerLimit[j] ? lowerLimit[j] : lower[j]);
			}
		}
	}

	return true;
}


bool RetSegm::OptimalPath::imposePathConstraintsForIOS(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, int upperRange, int lowerRange, int moveSpan)
{
	auto inner = layerInn->getPoints();
	auto outer = layerOut->getPoints();
	int size = (int)inner.size();
	int height = imgSrc->getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, moveSpan);

	auto upperLimit = vector<int>(size, 0);
	auto lowerLimit = vector<int>(size, height - 1);
	auto innerDelta = layerInn->getDeltas();

	// Limit upper range to between inner and outer boundary. 
	for (int i = 0; i < size; i++) {
		if (inner[i].valid) {
			if (outer[i].valid) {
				upperLimit[i] = inner[i].y + (outer[i].y - inner[i].y) / 2;
			}
			else {
				upperLimit[i] = inner[i].y;
			}
		}
	}

	// Limit lower range. 
	// For IOS, the lower range should be more than this limitation. (Minimum of range)
	for (int i = 0; i < size; i++) {
		if (inner[i].valid) {
			if (outer[i].valid) {
				lowerLimit[i] = inner[i].y;
			}
			else {
				lowerLimit[i] = inner[i].y + lowerRange;	// ?? 
			}
		}
	}

	// Set the movable range considering the deviation of outer pointers nearby. 
	int devHalfSize = 5;
	int ymax, ymin, diff, sidx, eidx;
	bool found;
	for (int i = 0; i < size; i++) {
		ymax = -1; ymin = height;
		found = false;

		sidx = i - devHalfSize;
		eidx = i + devHalfSize;
		sidx = (sidx < 0 ? 0 : sidx);
		eidx = (eidx >= size ? size - 1 : eidx);
		for (int k = sidx; k <= eidx; k++) {
			if (outer[i].valid) {
				ymax = (ymax < outer[i].y ? outer[i].y : ymax);
				ymin = (ymin > outer[i].y ? outer[i].y : ymin);
				found = true;
			}
		}
		if (found) {
			diff = (ymax - ymin);
			delta[i] = (diff > delta[i] ? diff : delta[i]);
		}
		else {
			// If there are no valid outer points, borrows the inner deltas.  
			delta[i] = (innerDelta[i] > delta[i] ? innerDelta[i] : delta[i]);
		}
	}

	int last = -1;
	int dist, yval, temp;
	// float rate;

	for (int i = 0; i < size; i++) {
		if (outer[i].valid) {
			dist = i - last;
			if (dist > 1) {
				if (last >= 0) {
					// rate = (float)(outer[i].getY() - outer[last].getY()) / dist;
					ymax = outer[i].y;
					ymin = outer[last].y;
					if (ymax < ymin) {
						temp = ymax; ymax = ymin; ymin = temp;
					}

					for (int j = last + 1, k = 1; j < i; j++, k++) {
						// yval = (int)(outer[last].getY() + rate*k);
						upper[j] = ymin - upperRange;
						lower[j] = ymax + lowerRange;
						// delta[j] = (int)rate + 1;
						upper[j] = (upper[j] < upperLimit[j] ? upperLimit[j] : upper[j]);
						lower[j] = (lower[j] < lowerLimit[j] ? lowerLimit[j] : lower[j]);
					}
				}
				else {
					yval = outer[i].y;
					for (int j = 0; j < i; j++) {
						upper[j] = yval - lowerRange;
						lower[j] = yval + lowerRange;
						upper[j] = (upper[j] < upperLimit[j] ? upperLimit[j] : upper[j]);
						lower[j] = (lower[j] < lowerLimit[j] ? lowerLimit[j] : lower[j]);
					}
				}
			}

			yval = outer[i].y;
			upper[i] = yval - upperRange;
			lower[i] = yval + lowerRange;
			upper[i] = (upper[i] < upperLimit[i] ? upperLimit[i] : upper[i]);
			lower[i] = (lower[i] < lowerLimit[i] ? lowerLimit[i] : lower[i]);
			last = i;
		}
	}

	if (outer[size - 1].valid == false) {
		if (last >= 0) {
			yval = (int)outer[last].y;
			for (int j = last + 1; j < size; j++) {
				upper[j] = yval - lowerRange;
				lower[j] = yval + lowerRange;
				upper[j] = (upper[j] < upperLimit[j] ? upperLimit[j] : upper[j]);
				lower[j] = (lower[j] < lowerLimit[j] ? lowerLimit[j] : lower[j]);
			}
		}
	}

	return true;
}


bool RetSegm::OptimalPath::createPathCostMap(const SegmImage * srcImg, std::vector<int> upper, std::vector<int> lower, std::vector<int> delta, float thresh1, float thresh2, int edgeSpan, bool ascent, SegmImage * costImg)
{
	if (srcImg->getByteSize() <= 0) {
		return false;
	}

	const Mat srcMat = srcImg->getCvMatConst();
	Mat outMat = Mat::zeros(srcMat.size(), CV_32SC1);
	outMat.setTo(OPTIMAL_PATH_COST_MAX);

	int valMin = (int)(thresh1 * 255);
	int valMax = (int)(thresh2 * 255);

	int span1, span2, sum1, sum2, cnt1, cnt2, val1, val2, cost;

	for (int c = 0; c < srcMat.cols; c++) {
		// Note that upper and lower boundaries are adjusted and confirmed that its range is valid. 
		for (int r = upper[c]; r <= lower[c]; r++) {
			span1 = r - edgeSpan;
			span2 = r + edgeSpan;
			span1 = (span1 < 0 ? 0 : span1);
			span2 = (span2 >= srcMat.rows ? srcMat.rows - 1 : span2);

			sum1 = sum2 = cnt1 = cnt2 = 0;
			for (int k = span1; k < r; k++) {
				sum1 += srcMat.at<uchar>(k, c);
				cnt1++;
			}
			for (int k = r + 1; k <= span2; k++) {
				sum2 += srcMat.at<uchar>(k, c);
				cnt2++;
			}

			if (cnt1 <= 0 || cnt2 <= 0) {
				continue;
			}

			// Restrict the intensity on the side of edge to the given threshold range. 
			val1 = sum1 / cnt1;
			val1 = (val1 < valMin ? valMin : val1);
			val1 = (val1 > valMax ? valMax : val1);
			val2 = sum2 / cnt2;
			val2 = (val2 < valMin ? valMin : val2);
			val2 = (val2 > valMax ? valMax : val2);

			// Set an element as the amount of edge gradient ascent. 
			if (ascent) {
				cost = (val1 - val2);
			}
			else {
				cost = (val2 - val1);
			}

			// Cost at the rightmost edge of interest should have an integer with negative sign, 
			// this makes computational burden be dodged by managing cost elements with float. 
			outMat.at<int>(r, c) = cost;
		}
	}

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	int minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = OPTIMAL_PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				if (k >= 0 && k < outMat.rows) {
					if (outMat.at<int>(k, c - 1) < minCost) {
						minCost = outMat.at<int>(k, c - 1);
					}
				}
			}
			outMat.at<int>(r, c) += minCost;
			//line += to_string(outMat.at<int>(r, c));
			//line += " ";
		}
		//DebugOut() << line;
	}

	costImg->getCvMat() = outMat;
	return true;
}


bool RetSegm::OptimalPath::searchPathMinCost(const SegmImage * costImg, std::vector<int> upper, std::vector<int> lower, std::vector<int> delta, SegmLayer * layerOut)
{
	if (costImg->getByteSize() <= 0) {
		return false;
	}

	const Mat costMat = costImg->getCvMatConst();
	Mat pathMat = Mat(1, costMat.cols, CV_32SC1);

	// Starting from the right most column. 
	int lastIdx = -1;
	int minCost = OPTIMAL_PATH_COST_MAX;
	int r, c;
	c = costMat.cols - 1;
	for (r = upper[c]; r < lower[c]; r++) {
		if (costMat.at<int>(r, c) <= minCost) {
			minCost = costMat.at<int>(r, c);
			lastIdx = r;
		}
	}
	pathMat.at<int>(0, c) = lastIdx;

	// Follow the minimum cost path in reverse order. 
	// Next point should be within the allowed vertical distance. 
	int rBeg, rEnd;
	for (c = costMat.cols - 2; c >= 0; c--) {
		rBeg = lastIdx - delta[c];
		rEnd = lastIdx + delta[c];
		rBeg = (rBeg < 0 ? 0 : rBeg);
		rEnd = (rEnd >= costMat.rows ? costMat.rows - 1 : rEnd);

		// Note that if any element with cost calculated from intensities (less than maximum at default) 
		// is not found at this column, the last index would go down toward the bottom of map. 
		minCost = OPTIMAL_PATH_COST_MAX;
		for (r = rBeg; r <= rEnd; r++) {
			if (costMat.at<int>(r, c) <= minCost) {
				minCost = costMat.at<int>(r, c);
				lastIdx = r;
			}
		}
		pathMat.at<int>(0, c) = lastIdx;
	}

	layerOut->initialize(pathMat);
	return true;
}











// Deprecated from here.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RetSegm::OptimalPath::findBoundaryILM(const ImageMat & srcImg, Boundary& innerBound, Boundary& outerBound, ImageMat & costImg, Boundary & result)
{
	const Mat srcMat = srcImg.getCvMatConst();
	innerBound.resize(srcImg.getWidth(), srcImg.getHeight());
	outerBound.resize(srcImg.getWidth(), srcImg.getHeight());

	vector<int> upperEnds, lowerEnds, moveDelta;

	float sampleRatio = srcImg.getSampleRatioHeight();
	int upperRange = (int)(PATH_ILM_UPPER_RANGE*sampleRatio);
	int lowerRange = (int)(PATH_ILM_LOWER_RANGE*sampleRatio);
	int moveSpan = (int)(PATH_ILM_CHANGE_MAX*sampleRatio);
	int edgeSize = (int)(PATH_ILM_EDGE_SIZE*sampleRatio);

	imposePathConstraintsForILM(srcImg, innerBound, outerBound, upperEnds, lowerEnds, moveDelta, upperRange, lowerRange, moveSpan);
	result.m_deltas = moveDelta;

	createPathCostMap(srcImg, upperEnds, lowerEnds, moveDelta, PATH_ILM_THRESH1, PATH_ILM_THRESH2, edgeSize, true, costImg);
	searchPathMinCost(costImg, upperEnds, lowerEnds, moveDelta, result);

	sampleRatio = srcImg.getSampleRatioWidth();
	int smoothWidth = (int)(PATH_ILM_SMOOTH_WIDTH*sampleRatio);
	int smoothDegree = PATH_ILM_SMOOTH_DEGREE;
	auto outs = CppUtil::SgFilter::smoothInts(result.getPointYs(), smoothWidth, smoothDegree);
	result.createPoints(outs, srcImg.getWidth(), srcImg.getHeight());

	return true;
}

bool RetSegm::OptimalPath::findBoundaryIOS(const ImageMat & srcImg, Boundary & innerBound, Boundary & outerBound, ImageMat & costImg, Boundary & result)
{
	const Mat srcMat = srcImg.getCvMatConst();

	vector<int> upperEnds, lowerEnds, moveDelta;

	float sampleRatio = srcImg.getSampleRatioHeight();
	int upperRange = (int)(PATH_IOS_UPPER_RANGE*sampleRatio);
	int lowerRange = (int)(PATH_IOS_LOWER_RANGE*sampleRatio);
	int moveSpan = (int)(PATH_IOS_CHANGE_MAX*sampleRatio);
	int edgeSize = (int)(PATH_IOS_EDGE_SIZE*sampleRatio);

	imposePathConstraintsForIOS(srcImg, innerBound, outerBound, upperEnds, lowerEnds, moveDelta, upperRange, lowerRange, moveSpan);
	createPathCostMap(srcImg, upperEnds, lowerEnds, moveDelta, PATH_IOS_THRESH1, PATH_IOS_THRESH2, edgeSize, true, costImg);
	searchPathMinCost(costImg, upperEnds, lowerEnds, moveDelta, result);

	sampleRatio = srcImg.getSampleRatioWidth();
	int smoothWidth = (int)(PATH_IOS_SMOOTH_WIDTH*sampleRatio);
	int smoothDegree = PATH_IOS_SMOOTH_DEGREE;
	auto outs = CppUtil::SgFilter::smoothInts(result.getPointYs(), smoothWidth, smoothDegree);
	result.createPoints(outs, srcImg.getWidth(), srcImg.getHeight());

	return true;
	/*
	auto inner = adjustPathUpperBound(srcImg, innerBound, PATH_IOS_UPPER_BOUND_SPAN);
	auto outer = adjustPathLowerBound(srcImg, outerBound, PATH_IOS_LOWER_BOUND_SPAN);
	auto delta = makePathChangeContraints(outerBound, PATH_IOS_DELTA_FACTOR);

	createPathCostMap(srcImg, inner, outer, delta, PATH_IOS_THRESH1, PATH_IOS_THRESH2, PATH_IOS_EDGE_SIZE, true, costImg);
	searchPathMinCost(costImg, inner, outer, delta, result);

	auto outs = CppUtil::SgFilter::smoothInts(result.getPointYs(), PATH_IOS_SMOOTH_WIDTH, PATH_IOS_SMOOTH_DEGREE);
	result.createPoints(outs);
	*/
	return true;
}

bool RetSegm::OptimalPath::findBoundaryToRPE(const ImageMat & srcImg, Boundary & innerBound, ImageMat & costImg, Boundary & boundRPE)
{
	const Mat srcMat = srcImg.getCvMatConst();

	auto inner = adjustPathLowerBound(srcImg, innerBound, PATH_RPE_UPPER_BOUND_SPAN);
	auto outer = adjustPathLowerBound(srcImg, innerBound, PATH_RPE_LOWER_BOUND_SPAN);
	auto delta = makePathChangeContraints(innerBound, PATH_RPE_DELTA_FACTOR);

	createPathCostMap(srcImg, inner, outer, delta, PATH_RPE_THRESH1, PATH_RPE_THRESH2, PATH_RPE_EDGE_SPAN_SIZE, false, costImg);
	searchPathMinCost(costImg, inner, outer, delta, boundRPE);

	auto outs = CppUtil::SgFilter::smoothInts(boundRPE.getPointYs(), PATH_RPE_SMOOTH_WIDTH, PATH_RPE_SMOOTH_DEGREE);
	boundRPE.createPoints(outs);
	return false;
}


bool RetSegm::OptimalPath::createPathCostMap(const ImageMat & srcImg, std::vector<int> upper, std::vector<int> lower, std::vector<int> delta,
	float thresh1, float thresh2, int edgeSpan, bool ascent, ImageMat & costImg)
{
	if (srcImg.getByteSize() <= 0) {
		return false;
	}

	const Mat srcMat = srcImg.getCvMatConst();
	Mat outMat = Mat::zeros(srcMat.size(), CV_32SC1);
	outMat.setTo(OPTIMAL_PATH_COST_MAX);

	int valMin = (int)(thresh1 * 255);
	int valMax = (int)(thresh2 * 255);

	int span1, span2, sum1, sum2, cnt1, cnt2, val1, val2, cost;

	for (int c = 0; c < srcMat.cols; c++) {
		// Note that upper and lower boundaries are adjusted and confirmed that its range is valid. 
		for (int r = upper[c]; r <= lower[c]; r++) {
			span1 = r - edgeSpan;
			span2 = r + edgeSpan;
			span1 = (span1 < 0 ? 0 : span1);
			span2 = (span2 >= srcMat.rows ? srcMat.rows - 1 : span2);

			sum1 = sum2 = cnt1 = cnt2 = 0;
			for (int k = span1; k < r; k++) {
				sum1 += srcMat.at<uchar>(k, c);
				cnt1++;
			}
			for (int k = r + 1; k <= span2; k++) {
				sum2 += srcMat.at<uchar>(k, c);
				cnt2++;
			}

			if (cnt1 <= 0 || cnt2 <= 0) {
				continue;
			}

			// Restrict the intensity on the side of edge to the given threshold range. 
			val1 = sum1 / cnt1;
			val1 = (val1 < valMin ? valMin : val1);
			val1 = (val1 > valMax ? valMax : val1);
			val2 = sum2 / cnt2;
			val2 = (val2 < valMin ? valMin : val2);
			val2 = (val2 > valMax ? valMax : val2);

			// Set an element as the amount of edge gradient ascent. 
			if (ascent) {
				cost = (val1 - val2);
			}
			else {
				cost = (val2 - val1);
			}

			// Cost at the rightmost edge of interest should have an integer with negative sign, 
			// this makes computational burden be dodged by managing cost elements with float. 
			outMat.at<int>(r, c) = cost;
		}
	}

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	int minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = OPTIMAL_PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				if (k >= 0 && k < outMat.rows) {
					if (outMat.at<int>(k, c - 1) < minCost) {
						minCost = outMat.at<int>(k, c - 1);
					}
				}
			}
			outMat.at<int>(r, c) += minCost;
			//line += to_string(outMat.at<int>(r, c));
			//line += " ";
		}
		//DebugOut() << line;
	}

	costImg.getCvMat() = outMat;
	return false;
}


bool RetSegm::OptimalPath::searchPathMinCost(const ImageMat & costImg, std::vector<int> upper, std::vector<int> lower, std::vector<int> delta, Boundary & outBound)
{
	if (costImg.getByteSize() <= 0) {
		return false;
	}

	const Mat costMat = costImg.getCvMatConst();
	Mat pathMat = Mat(1, costMat.cols, CV_32SC1);

	// Starting from the right most column. 
	int lastIdx = -1;
	int minCost = OPTIMAL_PATH_COST_MAX;
	int r, c;
	c = costMat.cols - 1;
	for (r = upper[c]; r < lower[c]; r++) {
		if (costMat.at<int>(r, c) <= minCost) {
			minCost = costMat.at<int>(r, c);
			lastIdx = r;
		}
	}
	pathMat.at<int>(0, c) = lastIdx;

	// Follow the minimum cost path in reverse order. 
	// Next point should be within the allowed vertical distance. 
	int rBeg, rEnd;
	for (c = costMat.cols - 2; c >= 0; c--) {
		rBeg = lastIdx - delta[c];
		rEnd = lastIdx + delta[c];
		rBeg = (rBeg < 0 ? 0 : rBeg);
		rEnd = (rEnd >= costMat.rows ? costMat.rows - 1 : rEnd);

		// Note that if any element with cost calculated from intensities (less than maximum at default) 
		// is not found at this column, the last index would go down toward the bottom of map. 
		minCost = OPTIMAL_PATH_COST_MAX;
		for (r = rBeg; r <= rEnd; r++) {
			if (costMat.at<int>(r, c) <= minCost) {
				minCost = costMat.at<int>(r, c);
				lastIdx = r;
			}
		}
		pathMat.at<int>(0, c) = lastIdx;
	}

	outBound.createPoints(pathMat);
	return true;
}


bool RetSegm::OptimalPath::imposePathConstraintsForILM(const ImageMat & srcImg, Boundary & innerBound, Boundary & outerBound,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
	int upperRange, int lowerRange, int moveSpan)
{
	auto inner = innerBound.getPoints();
	auto outer = outerBound.getPoints();
	int size = (int)inner.size();
	int height = srcImg.getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	auto upperLimit = vector<int>(size, 0);
	auto lowerLimit = vector<int>(size, height - 1);

	// Limit lower range to between inner and outer boundary. 
	for (int i = 0; i < size; i++) {
		if (inner[i].isValid() && outer[i].isValid()) {
			lowerLimit[i] = inner[i].getY() + (outer[i].getY() - inner[i].getY()) / 3;
		}
	}

	int last = -1;
	int dist, yval;
	float rate;
	for (int i = 0; i < size; i++) {
		if (inner[i].isValid()) {
			dist = i - last;
			if (dist > 1) {
				if (last >= 0) {
					rate = (float)(inner[i].getY() - inner[last].getY()) / dist;
					for (int j = last + 1, k = 1; j < i; j++, k++) {
						yval = (int)(inner[last].getY() + rate*k);
						upper[j] = yval - upperRange;
						lower[j] = yval + upperRange;
						delta[j] = (int)rate + 1;
						upper[j] = (upper[j] < 0 ? 0 : upper[j]);
						lower[j] = (lower[j] > lowerLimit[j] ? lowerLimit[j] : lower[j]);
						delta[j] = (delta[j] < moveSpan ? moveSpan : delta[j]);
					}
				}
				else {
					yval = inner[i].getY();
					for (int j = 0; j < i; j++) {
						upper[j] = yval - upperRange;
						lower[j] = yval + upperRange;
						delta[j] = moveSpan;
						upper[j] = (upper[j] < 0 ? 0 : upper[j]);
						lower[j] = (lower[j] > lowerLimit[j] ? lowerLimit[j] : lower[j]);
					}
				}
			}

			yval = inner[i].getY();
			upper[i] = yval - upperRange;
			lower[i] = yval + lowerRange;
			delta[i] = moveSpan;
			upper[i] = (upper[i] < 0 ? 0 : upper[i]);
			lower[i] = (lower[i] > lowerLimit[i] ? lowerLimit[i] : lower[i]);
			last = i;
		}
	}

	if (inner[size - 1].isValid() == false) {
		if (last >= 0) {
			yval = (int)inner[last].getY();
			for (int j = last + 1; j < size; j++) {
				upper[j] = yval - upperRange;
				lower[j] = yval + upperRange;
				delta[j] = moveSpan;
				upper[j] = (upper[j] < 0 ? 0 : upper[j]);
				lower[j] = (lower[j] > lowerLimit[j] ? lowerLimit[j] : lower[j]);
			}
		}
	}

	return true;
}


bool RetSegm::OptimalPath::imposePathConstraintsForIOS(const ImageMat & srcImg, Boundary & innerBound, Boundary & outerBound,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
	int upperRange, int lowerRange, int moveSpan)
{
	auto inner = innerBound.getPoints();
	auto outer = outerBound.getPoints();
	int size = (int)inner.size();
	int height = srcImg.getHeight();

	if (size <= 0 || height <= 0) {
		return false;
	}

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, moveSpan);

	auto upperLimit = vector<int>(size, 0);
	auto lowerLimit = vector<int>(size, height - 1);
	auto innerDelta = innerBound.m_deltas;

	// Limit upper range to between inner and outer boundary. 
	for (int i = 0; i < size; i++) {
		if (inner[i].isValid()) {
			if (outer[i].isValid()) {
				upperLimit[i] = inner[i].getY() + (outer[i].getY() - inner[i].getY()) / 2;
			}
			else {
				upperLimit[i] = inner[i].getY();
			}
		}
	}

	// Limit lower range. 
	// For IOS, the lower range should be more than this limitation. (Minimum of range)
	for (int i = 0; i < size; i++) {
		if (inner[i].isValid()) {
			if (outer[i].isValid()) {
				lowerLimit[i] = inner[i].getY();
			}
			else {
				lowerLimit[i] = inner[i].getY() + lowerRange;	// ?? 
			}
		}
	}

	// Set the movable range considering the deviation of outer pointers nearby. 
	int devHalfSize = 5;
	int ymax, ymin, diff, sidx, eidx;
	bool found;
	for (int i = 0; i < size; i++) {
		ymax = -1; ymin = height;
		found = false;

		sidx = i - devHalfSize;
		eidx = i + devHalfSize;
		sidx = (sidx < 0 ? 0 : sidx);
		eidx = (eidx >= size ? size - 1 : eidx);
		for (int k = sidx; k <= eidx; k++) {
			if (outer[i].isValid()) {
				ymax = (ymax < outer[i].getY() ? outer[i].getY() : ymax);
				ymin = (ymin > outer[i].getY() ? outer[i].getY() : ymin);
				found = true;
			}
		}
		if (found) {
			diff = (ymax - ymin);
			delta[i] = (diff > delta[i] ? diff : delta[i]);
		}
		else {
			// If there are no valid outer points, borrows the inner deltas.  
			delta[i] = (innerDelta[i] > delta[i] ? innerDelta[i] : delta[i]);
		}
	}

	int last = -1;
	int dist, yval, temp;
	// float rate;

	for (int i = 0; i < size; i++) {
		if (outer[i].isValid()) {
			dist = i - last;
			if (dist > 1) {
				if (last >= 0) {
					// rate = (float)(outer[i].getY() - outer[last].getY()) / dist;
					ymax = outer[i].getY();
					ymin = outer[last].getY();
					if (ymax < ymin) {
						temp = ymax; ymax = ymin; ymin = temp;
					}

					for (int j = last + 1, k = 1; j < i; j++, k++) {
						// yval = (int)(outer[last].getY() + rate*k);
						upper[j] = ymin - upperRange;
						lower[j] = ymax + lowerRange;
						// delta[j] = (int)rate + 1;
						upper[j] = (upper[j] < upperLimit[j] ? upperLimit[j] : upper[j]);
						lower[j] = (lower[j] < lowerLimit[j] ? lowerLimit[j] : lower[j]);
					}
				}
				else {
					yval = outer[i].getY();
					for (int j = 0; j < i; j++) {
						upper[j] = yval - lowerRange;
						lower[j] = yval + lowerRange;
						upper[j] = (upper[j] < upperLimit[j] ? upperLimit[j] : upper[j]);
						lower[j] = (lower[j] < lowerLimit[j] ? lowerLimit[j] : lower[j]);
					}
				}
			}

			yval = outer[i].getY();
			upper[i] = yval - upperRange;
			lower[i] = yval + lowerRange;
			upper[i] = (upper[i] < upperLimit[i] ? upperLimit[i] : upper[i]);
			lower[i] = (lower[i] < lowerLimit[i] ? lowerLimit[i] : lower[i]);
			last = i;
		}
	}

	if (outer[size - 1].isValid() == false) {
		if (last >= 0) {
			yval = (int)outer[last].getY();
			for (int j = last + 1; j < size; j++) {
				upper[j] = yval - lowerRange;
				lower[j] = yval + lowerRange;
				upper[j] = (upper[j] < upperLimit[j] ? upperLimit[j] : upper[j]);
				lower[j] = (lower[j] < lowerLimit[j] ? lowerLimit[j] : lower[j]);
			}
		}
	}

	return true;
}


std::vector<int> RetSegm::OptimalPath::makePathChangeContraints(const Boundary & inBound, float factor)
{
	auto values = vector<int>(inBound.getSize(), OPTIMAL_PATH_CHANGE_MAX);
	auto edges = inBound.getPoints();

	int preY = -1;
	int delta;
	for (int i = 0; i < edges.size(); i++) {
		if (edges[i].isValid()) {
			if (preY >= 0) {
				delta = (int)(abs(edges[i].getY() - preY) * factor);
				delta = (delta < OPTIMAL_PATH_CHANGE_MAX ? OPTIMAL_PATH_CHANGE_MAX : delta);
				values[i] = delta;
			}
			preY = edges[i].getY();
		}
	}

	return values;
}

vector<int> RetSegm::OptimalPath::adjustPathUpperBound(const ImageMat & srcImg, Boundary & inBound, int span)
{
	const Mat srcMat = srcImg.getCvMatConst();

	// Initial boundary points.
	auto edges = inBound.getPoints();
	auto values = vector<int>(edges.size());

	int locY;
	for (int c = 0; c < edges.size(); c++) {
		if (edges[c].isValid()) {
			locY = edges[c].getY() - span;
			if (locY < 0 || locY >= srcMat.rows) {
				locY = 0;
			}
		}
		else {
			locY = 0;
		}
		values[c] = locY;
	}

	return values;
}


vector<int> RetSegm::OptimalPath::adjustPathLowerBound(const ImageMat & srcImg, Boundary & inBound, int span)
{
	const Mat srcMat = srcImg.getCvMatConst();

	// Initial boundary points.
	auto edges = inBound.getPoints();
	auto values = vector<int>(edges.size());

	int locY;
	for (int c = 0; c < edges.size(); c++) {
		if (edges[c].isValid()) {
			locY = edges[c].getY() + span;
			if (locY < 0 || locY >= srcMat.rows) {
				locY = srcMat.rows - 1;
			}
		}
		else {
			locY = srcMat.rows - 1;
		}
		values[c] = locY;
	}

	return values;
}

