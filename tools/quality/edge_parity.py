#!/usr/bin/env python3
"""Orientation-aware structural edge comparison for camera-matched renders."""

import numpy as np
from PIL import Image
from scipy.ndimage import distance_transform_edt, gaussian_filter, sobel


LUMA = np.array([0.2126, 0.7152, 0.0722])


def detect_edges(rgb, sigma=1.5, threshold=10.0):
    pixels = np.asarray(rgb, dtype=np.float64)
    if pixels.ndim != 3 or pixels.shape[2] != 3:
        raise ValueError("edge input must be RGB")
    luminance = gaussian_filter(pixels @ LUMA, sigma)
    dx = sobel(luminance, axis=1) / 8.0
    dy = sobel(luminance, axis=0) / 8.0
    strength = np.hypot(dx, dy)
    # Gradient orientation is edge-normal orientation modulo 180 degrees.
    orientation = np.floor((np.arctan2(dy, dx) + np.pi / 8) /
                           (np.pi / 4)).astype(np.int8) % 4
    return strength >= threshold, orientation


def matched_edges(source, source_orientation, target, target_orientation, tolerance):
    matched = np.zeros_like(source)
    for direction in range(4):
        selected = source & (source_orientation == direction)
        candidates = target & (target_orientation == direction)
        if selected.any() and candidates.any():
            distance = distance_transform_edt(~candidates)
            matched[selected] = distance[selected] <= tolerance
    return matched


def edge_parity(reference, candidate, regions=None, tolerance=3, sigma=1.5,
                threshold=10.0):
    if np.shape(reference) != np.shape(candidate):
        raise ValueError("edge parity requires aligned images of equal size")
    ref, ref_orientation = detect_edges(reference, sigma, threshold)
    game, game_orientation = detect_edges(candidate, sigma, threshold)
    if not ref.any() or not game.any():
        raise ValueError("edge parity requires nonempty reference and candidate edges")
    ref_matched = matched_edges(ref, ref_orientation, game, game_orientation, tolerance)
    game_matched = matched_edges(game, game_orientation, ref, ref_orientation, tolerance)

    def score(box):
        x0, y0, x1, y1 = box
        area = np.s_[y0:y1, x0:x1]
        ref_count = int(ref[area].sum())
        game_count = int(game[area].sum())
        if not ref_count or not game_count:
            return {"reference_edges": ref_count, "candidate_edges": game_count,
                    "precision": 0.0, "recall": 0.0, "f1": 0.0}
        precision = float(game_matched[area].sum() / game_count)
        recall = float(ref_matched[area].sum() / ref_count)
        f1 = 2 * precision * recall / (precision + recall) if precision + recall else 0.0
        return {"reference_edges": ref_count, "candidate_edges": game_count,
                "precision": precision, "recall": recall, "f1": f1}

    height, width = ref.shape
    results = {"algorithm": "gaussian-sobel-orientation-distance/v1",
               "sigma": sigma, "gradient_threshold": threshold,
               "tolerance_pixels": tolerance, "whole_frame": score((0, 0, width, height)),
               "regions": {name: score(box) for name, box in (regions or {}).items()}}
    return results, ref, game


def edge_overlay(reference, ref_edges, candidate_edges):
    gray = (np.asarray(reference, dtype=np.float32) @ LUMA * 0.25).astype(np.uint8)
    rgb = np.repeat(gray[:, :, None], 3, axis=2)
    rgb[ref_edges] = (255, 70, 70)
    rgb[candidate_edges] = (50, 190, 255)
    rgb[ref_edges & candidate_edges] = (255, 255, 255)
    return Image.fromarray(rgb)
