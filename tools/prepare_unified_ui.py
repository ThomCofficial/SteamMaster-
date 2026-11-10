#!/usr/bin/env python3
"""Prepare JUCE-ready artwork from the user's unchanged, approved source panels.

This is deterministic build-time compositing, not an image-generation redesign.
Every moving rotor is extracted from its source module, masked to the rotor core,
and packed into a PNG atlas. The underlying rotor is removed from the faceplate
with locally constrained inpainting. The original assets remain untouched.
"""
from __future__ import annotations
import json
from pathlib import Path
import cv2
import numpy as np
from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "plugin" / "assets"
OUT = ROOT / "plugin" / "generated"
OUT.mkdir(exist_ok=True, parents=True)
LAYOUT = json.loads((ROOT / "tools" / "ui_layout.json").read_text())

CELL = 160
COLS = 8

def image(path: Path) -> Image.Image:
    return Image.open(path).convert("RGBA")

def save(image_array: np.ndarray, name: str) -> None:
    Image.fromarray(image_array, "RGBA").save(OUT / name, optimize=True)

def prepare_plate_and_atlas(name: str, meta: dict) -> None:
    art = image(ASSETS / meta["source"])
    rgb = np.asarray(art).copy()
    h, w = rgb.shape[:2]
    knobs = [p for p in meta["controls"] if p["kind"] == "knob"]
    atlas = np.zeros((((len(knobs)+COLS-1)//COLS)*CELL,COLS*CELL,4),np.uint8)
    cut_mask = np.zeros((h,w),np.uint8)
    for idx, p in enumerate(knobs):
        x,y,r = float(p["x"]),float(p["y"]),float(p["r"])
        if x < 1 or y < 1 or x >= w or y >= h:
            raise ValueError(f"Out-of-bounds knob {name}:{p['id']} at {x},{y} inside {w}x{h}")
        # Keep the original hardware surface, avoid rotating engraved scales.
        core = max(10.0,r*0.84)
        margin = max(1.7,r*.055)
        yy,xx=np.ogrid[:h,:w]
        dist=np.sqrt((xx-x)**2+(yy-y)**2)
        alpha=np.clip((core-dist)/margin,0,1)
        # Draw in a padded sprite with a precise original pixel center.
        sy,sx=divmod(idx,COLS)
        x0=max(0,int(x-CELL/2)); y0=max(0,int(y-CELL/2))
        x1=min(w,int(x+CELL/2)); y1=min(h,int(y+CELL/2))
        destx=sx*CELL+int(CELL/2-(x-x0))
        desty=sy*CELL+int(CELL/2-(y-y0))
        region=rgb[y0:y1,x0:x1].copy()
        region[:,:,3]=np.uint8(alpha[y0:y1,x0:x1]*region[:,:,3])
        dx0=max(0,destx);dy0=max(0,desty)
        dx1=min(atlas.shape[1],destx+region.shape[1])
        dy1=min(atlas.shape[0],desty+region.shape[0])
        atlas[dy0:dy1,dx0:dx1]=region[dy0-desty:dy1-desty,dx0-destx:dx1-destx]
        # Conservatively erase only baked rotor body; do not remove printed rings.
        cut_mask=np.maximum(cut_mask,np.uint8(dist<=core*.98)*255)
    if name == "tape":
        # The two real photographed reels are separated from the dark wood
        # backing; only those source reels rotate, never the entire panel.
        reels=np.zeros((256,512,4),np.uint8)
        for ri,(rx,ry) in enumerate(((395,141),(1151,141))):
            rr=114.0
            yy,xx=np.ogrid[:h,:w]
            dd=np.sqrt((xx-rx)**2+(yy-ry)**2)
            aa=np.clip((rr-dd)/2.5,0,1)
            x0=max(0,rx-128);x1=min(w,rx+128)
            y0=max(0,ry-128);y1=min(h,ry+128)
            cropped=rgb[y0:y1,x0:x1].copy()
            cropped[:,:,3]=np.uint8(cropped[:,:,3]*aa[y0:y1,x0:x1])
            dx=ri*256+128-(rx-x0);dy=128-(ry-y0)
            reels[dy:dy+cropped.shape[0],dx:dx+cropped.shape[1]]=cropped
            cut_mask=np.maximum(cut_mask,np.uint8(dd<=rr*.97)*255)
        save(reels,"tape_reels.png")
    # RGB inpaint preserves nearby parchment, wood and metal texture better than
    # compositing the original baked rotor beneath an independent rotating rotor.
    bgr=cv2.cvtColor(rgb[:,:,:3],cv2.COLOR_RGB2BGR)
    if np.any(cut_mask):
        bgr=cv2.inpaint(bgr,cut_mask,6,cv2.INPAINT_TELEA)
    face=np.empty_like(rgb)
    face[:,:,:3]=cv2.cvtColor(bgr,cv2.COLOR_BGR2RGB)
    face[:,:,3]=rgb[:,:,3]
    save(face,f"{name}_faceplate.png")
    save(atlas,f"{name}_rotors.png")
    print(f"{name}: original {w}x{h}, {len(knobs)} independent rotors; faceplate/atlas ready")

for name,meta in LAYOUT["modules"].items():
    prepare_plate_and_atlas(name,meta)

# Global Header: restore source labels/scales, unlike the previous blank_candidate.
# Only inpaint the two silver dial cores. The independently animated buttons in
# HeaderModule.cpp cover their original positions using the approved state images.
raw=np.asarray(image(ASSETS / "SteamMaster_Global_Header_Complete_v2" /
                     "header_source_reference.png")).copy()
h,w=raw.shape[:2]
mask=np.zeros((h,w),np.uint8)
for x,y in ((283,417),(466,417)):
    cv2.circle(mask,(x,y),50,255,-1)
bgr=cv2.inpaint(cv2.cvtColor(raw[:,:,:3],cv2.COLOR_RGB2BGR),mask,5,cv2.INPAINT_TELEA)
raw[:,:,:3]=cv2.cvtColor(bgr,cv2.COLOR_BGR2RGB)
save(raw,"header_original_faceplate.png")
print("Global: source labels and physical scales preserved, dial cores isolated")
