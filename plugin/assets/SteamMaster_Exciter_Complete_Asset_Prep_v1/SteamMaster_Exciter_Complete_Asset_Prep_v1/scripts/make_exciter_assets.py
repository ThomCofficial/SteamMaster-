from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter, ImageFont
import cv2, numpy as np, json, zipfile, shutil, math, textwrap

SOURCE=Path('/mnt/data/1000031975.png')
ROOT=Path('/mnt/data/SteamMaster_Exciter_Complete_Asset_Prep_v1')
for sub in ['source','rotors_REFERENCE','buttons_REFERENCE','decor','meter','masks','mapping','qa','scripts']:
 (ROOT/sub).mkdir(parents=True,exist_ok=True)

src=Image.open(SOURCE).convert('RGB')
W,H=src.size
assert (W,H)==(1536,512),(W,H)
src.save(ROOT/'source'/'Exciter_original_1536x512.png')
# Preserve 1:1 pixels. Source's actual physical panel begins around y=62.
panel_bbox=(0,60,1536,462)
panel=src.crop(panel_bbox)
panel.save(ROOT/'source'/'Exciter_panel_uncorrected_1536x402.png')

font='/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'
fontbold='/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'
F=lambda size,bold=False: ImageFont.truetype(fontbold if bold else font,size)

# Model coordinates are relative to the user's supplied 1536x512 image.
# These are provisional measured faceplate centers, with engineering verification required before UI lock.
controls=[
 {'key':'mode_tube','type':'mode_button','cx':123,'cy':269,'w':143,'h':39,'state_in_image':'active','id':'exciter_mode','value':'Tube'},
 {'key':'mode_tape','type':'mode_button','cx':123,'cy':313,'w':143,'h':38,'state_in_image':'inactive','id':'exciter_mode','value':'Tape'},
 {'key':'mode_transformer','type':'mode_button','cx':123,'cy':354,'w':143,'h':38,'state_in_image':'inactive','id':'exciter_mode','value':'Transformer'},
 {'key':'module_in','type':'button','cx':122,'cy':398,'w':80,'h':39,'state_in_image':'active','id':'exciter_in'},
 {'key':'low_drive','type':'rotor','cx':290,'cy':320,'r':45,'id':'exciter_low_drive'},
 {'key':'low_freq','type':'rotor','cx':419,'cy':330,'r':34,'id':'exciter_low_freq'},
 {'key':'vu_meter','type':'meter','cx':633,'cy':305,'w':246,'h':208,'id':None},
 {'key':'high_drive','type':'rotor','cx':854,'cy':320,'r':46,'id':'exciter_high_drive'},
 {'key':'high_freq','type':'rotor','cx':986,'cy':332,'r':34,'id':'exciter_high_freq'},
 {'key':'crossover','type':'rotor','cx':1148,'cy':330,'r':34,'id':'exciter_crossover'},
 {'key':'output','type':'rotor','cx':1301,'cy':331,'r':33,'id':'exciter_output_db'},
 {'key':'mix','type':'rotor','cx':1448,'cy':329,'r':34,'id':'exciter_mix'},
 {'key':'left_decorative_lamp','type':'lamp','cx':219,'cy':137,'w':77,'h':126,'id':None},
 {'key':'right_decorative_lamp','type':'lamp','cx':1315,'cy':137,'w':75,'h':126,'id':None},
]

# Export all physical control appearance references individually. Circular alpha is confined
# to the rotating *interior* of each control; printed scales remain separate/static.
for c in controls:
 if c['type']=='rotor':
  r=c['r']; cx=c['cx']; cy=c['cy']; pad=2
  box=(cx-r-pad,cy-r-pad,cx+r+pad+1,cy+r+pad+1)
  crop=src.crop(box).convert('RGBA')
  m=Image.new('L',crop.size,0); md=ImageDraw.Draw(m)
  md.ellipse((pad,pad,2*r+pad,2*r+pad),fill=255)
  m=m.filter(ImageFilter.GaussianBlur(0.8))
  crop.putalpha(m)
  crop.save(ROOT/'rotors_REFERENCE'/f"{c['key']}_rotor_visual_reference.png")
 elif c['type']=='mode_button' or c['type']=='button':
  box=(int(c['cx']-c['w']/2),int(c['cy']-c['h']/2),int(c['cx']+c['w']/2),int(c['cy']+c['h']/2))
  crop=src.crop(box).convert('RGBA')
  mask=Image.new('L',crop.size,0)
  md=ImageDraw.Draw(mask)
  md.rounded_rectangle((1,1,crop.width-2,crop.height-2),radius=8,fill=255)
  mask=mask.filter(ImageFilter.GaussianBlur(0.7))
  crop.putalpha(mask)
  crop.save(ROOT/'buttons_REFERENCE'/f"{c['key']}_{c.get('state_in_image','')}_REFERENCE.png")
 elif c['type']=='lamp':
  box=(int(c['cx']-c['w']/2),int(c['cy']-c['h']/2),int(c['cx']+c['w']/2),int(c['cy']+c['h']/2))
  crop=src.crop(box).convert('RGBA')
  mask=Image.new('L',crop.size,0)
  md=ImageDraw.Draw(mask)
  md.ellipse((1,1,crop.width-2,crop.height-2), fill=255)
  mask=mask.filter(ImageFilter.GaussianBlur(1.1));crop.putalpha(mask)
  crop.save(ROOT/'decor'/f"{c['key']}_REFERENCE.png")

# VU full meter image reference, including beveled frame. No claim the needle is already separated.
vu_box=(502,197,777,426)
src.crop(vu_box).save(ROOT/'meter'/'vu_meter_original_needle_baked_REFERENCE.png')
# Standalone meter art and dynamic needle must be implemented as independent JUCE components.

# Full-panel masks: everything requiring visual replacement is recorded nondestructively.
mask=Image.new('L',src.size,0); md=ImageDraw.Draw(mask)
for c in controls:
 if c['type']=='rotor':
  x,y,r=c['cx'],c['cy'],c['r']
  md.ellipse((x-r-5,y-r-5,x+r+5,y+r+5),fill=255)
 elif c['type']=='mode_button' or c['type']=='button':
  w,h=c['w'],c['h'];x,y=c['cx'],c['cy']
  md.rounded_rectangle((x-w/2-3,y-h/2-2,x+w/2+3,y+h/2+2),radius=9,fill=255)
# Do not remove the meter frame; it remains part of the approved design.
mask.save(ROOT/'masks'/'interactive_controls_removal_mask.png')
# Include clear visual mask, *not* a fraudulent claim to have reconstructed concealed faceplate pixels.
vis=src.copy().convert('RGBA'); translucent=Image.new('RGBA',src.size,(0,0,0,0))
red=Image.new('RGBA',src.size,(242,55,44,98)); translucent.paste(red,(0,0),mask)
vis=Image.alpha_composite(vis,translucent); vis.save(ROOT/'qa'/'interactive_areas_marked.png')

# Deterministic proof-of-concept background inpainting. This is not ready as the final production faceplate.
# Keep only as a guide to regions needing actual reconstructed texture.
cv_src=cv2.cvtColor(np.array(src),cv2.COLOR_RGB2BGR)
small_mask=np.array(mask)
inpaint=cv2.inpaint(cv_src,small_mask,5,cv2.INPAINT_TELEA)
Image.fromarray(cv2.cvtColor(inpaint,cv2.COLOR_BGR2RGB)).save(ROOT/'masks'/'faceplate_INPAINT_TEST_NOT_PRODUCTION.png')

# DSP parameter registry (inspected directly from the active GitHub branch)
spec={
 'exciter_in':{'kind':'bool','default':True},
 'exciter_mode':{'kind':'choice','choices':['Tube','Tape','Transformer'],'default':'Tube'},
 'exciter_low_drive':{'kind':'float','min':0,'max':20,'default':0,'step':0.001,'unit':''},
 'exciter_low_freq':{'kind':'float','min':20,'max':800,'default':100,'step':0.001,'unit':'Hz'},
 'exciter_high_drive':{'kind':'float','min':0,'max':20,'default':0,'step':0.001,'unit':''},
 'exciter_high_freq':{'kind':'float','min':1000,'max':18000,'default':5000,'step':0.001,'unit':'Hz'},
 'exciter_crossover':{'kind':'float','min':100,'max':5000,'default':800,'step':0.001,'unit':'Hz'},
 'exciter_output_db':{'kind':'float','min':-12,'max':12,'default':0,'step':0.001,'unit':'dB'},
 'exciter_mix':{'kind':'float','min':0,'max':100,'default':100,'step':0.001,'unit':'%'},
}
# Full scales need all numbers drawn by JUCE, never copied from the AI image.
scale_rules={
 'low_drive':{'param':'exciter_low_drive','ticks':[0,5,10,15,20],'artwork_incorrect':'0–10'},
 'high_drive':{'param':'exciter_high_drive','ticks':[0,5,10,15,20],'artwork_incorrect':'0–10'},
 'low_freq':{'param':'exciter_low_freq','ticks':[20,200,400,600,800],'artwork_incorrect':'30/60/100/200/400 Hz'},
 'high_freq':{'param':'exciter_high_freq','ticks':[1000,5000,10000,15000,18000],'artwork_incorrect':'1.5/3/5/8/12 kHz'},
 'crossover':{'param':'exciter_crossover','ticks':[100,1000,2000,3000,5000],'artwork_incorrect':'250/400/800/1.2k/2k/3k Hz'},
 'output':{'param':'exciter_output_db','ticks':[-12,-6,0,6,12],'artwork_incorrect':None},
 'mix':{'param':'exciter_mix','ticks':[0,25,50,75,100],'artwork_incorrect':None},
}
# Use linear mappings to reflect current JUCE NormalisableRange; do not silently add skew.
manifest={
 'module':'Exciter','source':'1000031975.png','source_size':[1536,512],
 'physical_panel_crop':[0,60,1536,462],
 'source_of_truth':'Github: ThomCofficial/SteamMaster-, branch vst3/v1.7-performance-character, plugin/PluginProcessor.cpp lines 68-77',
 'status':'Art preserved. Control cutout references and masks are provisional; final clean faceplate requires reconstruction.',
 'units':'Pixels; centers measured in original image coordinate system, not crop-local.',
 'controls':controls,'parameters':spec,'scale_draw_rules':scale_rules,
 'meter':{'role':'Decorative VU gauge becoming animated','dsp_parameter_id':None,'status':'No verified GUI meter telemetry route; cannot claim live meter until added','note':'Do not invent an audio measurement or render a false responsive gauge.'},
 'lamps':{'left/right':'Decorative. Their illumination is not mapped to host automation or DSP unless a visual convention is later established.'}
}
(ROOT/'mapping'/'exciter_ui_to_dsp_manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf8')

# Precision-ready overlay for a future JUCE painter, colors only illustrative: labels are not baked onto decorative source.
# This preview is *not* an integrated interactive module.
qa=Image.new('RGB',(1576,1060),(15,17,19))
qa.paste(src,(20,55))
d=ImageDraw.Draw(qa)
d.text((26,17),'EXCITER — DSP-VERIFIED MODULE AUDIT  |  ORIGINAL ART KEPT',font=F(21,True),fill=(241,224,183))
for c in controls:
 if c['type'] in ['rotor','mode_button','button','meter']:
  x,y=c['cx']+20,c['cy']+55
  if c['type']=='rotor':
   r=c['r'];d.ellipse((x-r-2,y-r-2,x+r+2,y+r+2),outline=(12,248,168),width=3)
  elif c['type']=='meter':
   d.rectangle((x-120,y-98,x+120,y+100),outline=(253,186,67),width=3)
  else:
   d.rectangle((x-c['w']//2,y-c['h']//2,x+c['w']//2,y+c['h']//2),outline=(12,248,168),width=3)

orange=(255,194,98);white=(226,220,204)
start=600
d.text((30,start),'Parameter check against the existing VST3 DSP',font=F(25,True),fill=orange)
start+=44
labels=[
 ('MODE / IN','Tube · Tape · Transformer   |   independent IN bypass','MATCH'),
 ('LOW + HIGH DRIVE','Artwork 0–10  →  DSP 0–20','FIX UI'),
 ('LOW FREQ','Artwork 30–400 Hz  →  DSP 20–800 Hz','FIX UI'),
 ('HIGH FREQ','Artwork 1.5–12 kHz  →  DSP 1–18 kHz','FIX UI'),
 ('CROSSOVER','Artwork 250–3000 Hz  →  DSP 100–5000 Hz','FIX UI'),
 ('OUTPUT / MIX','−12 to +12 dB   |   0–100%','MATCH'),
 ('VU METER','Needle animation requires real DSP-side read-only telemetry','NOT WIRED')
]
for i,(a,b,status) in enumerate(labels):
 y=start+48*i
 d.text((32,y),a,font=F(18,True),fill=orange)
 d.text((275,y),b,font=F(18),fill=white)
 d.text((1410,y),status,font=F(16,True),fill=(135,255,195) if status=='MATCH' else (255,184,117))
qa.save(ROOT/'qa'/'Exciter_Whole_Module_QA.png')

# Sprite board: review extracted artwork at once, not a few isolated controls.
sprites=[]
for fname in sorted((ROOT/'rotors_REFERENCE').glob('*.png')):
 sprites.append((fname.stem,Image.open(fname).convert('RGBA')))
for fname in sorted((ROOT/'buttons_REFERENCE').glob('*.png')):
 sprites.append((fname.stem,Image.open(fname).convert('RGBA')))
for fname in sorted((ROOT/'decor').glob('*.png')):
 sprites.append((fname.stem,Image.open(fname).convert('RGBA')))
board=Image.new('RGB',(1536,680),(33,31,29)); dr=ImageDraw.Draw(board)
dr.text((20,12),'EXCITER — WHOLE-MODULE CONTROL APPEARANCE REFERENCES (NOT SEPARATED FACEPLATE)',font=F(20,True),fill=(240,213,159))
for i,(name,sp) in enumerate(sprites):
 col=i%5; row=i//5
 x=22+col*300; y=67+row*205
 block=Image.new('RGBA',(280,146),(57,53,44,255)); bd=ImageDraw.Draw(block)
 for yy in range(0,146,16):
  for xx in range(0,280,16):
   if ((xx+yy)//16)%2: bd.rectangle([xx,yy,xx+15,yy+15],fill=(74,70,63,255))
 sx=min(1.5,238/max(1,sp.width),132/max(1,sp.height))
 w,h=round(sp.width*sx),round(sp.height*sx)
 sp2=sp.resize((w,h),Image.Resampling.LANCZOS)
 block.alpha_composite(sp2,((280-w)//2,(146-h)//2))
 board.paste(block.convert('RGB'),(x,y))
 dr.text((x+2,y+153),name[:38],font=F(13),fill=(236,229,213))
board.save(ROOT/'qa'/'Exciter_All_Extracted_Control_Refs.png')

readme=f'''# SteamMaster — Exciter complete module handoff (asset preparation v1)

**Approved design: preserved. DSP: unchanged. GitHub: read-only. Cubase tests: not performed.**

## Material delivered

- Original **1536×512** user artwork, and full **1536×402** physical module crop.
- Reference cutouts for **all seven knob rotors** (2 drives, 2 frequencies, crossover, output, mix).
- Reference cutouts for three mode buttons plus IN, two decorative lamps, and the full VU meter.
- Full-size geometry/control-removal masks and a **read-only prototype** inpaint, labeled **NOT PRODUCTION**.
- Machine-readable coordinate manifest, verified parameter names/types/ranges/defaults, and exact scale data for JUCE rendering.
- Full-module QA review and control reference contact sheet.

## Double-check results

| Control | Current image | Actual processor DSP mapping | Action before JUCE release |
|---|---|---|---|
| MODE | Tube / Tape / Transformer | `exciter_mode`, same choices | Pass |
| IN | On/off | `exciter_in`, bool | Pass |
| LOW DRIVE | 0–10 printed | `exciter_low_drive`, **0–20** | **Replace scale in JUCE** |
| LOW FREQ | Printed 30–400 Hz | `exciter_low_freq`, **20–800 Hz** | **Replace scale in JUCE** |
| HIGH DRIVE | 0–10 printed | `exciter_high_drive`, **0–20** | **Replace scale in JUCE** |
| HIGH FREQ | Printed 1.5–12 kHz | `exciter_high_freq`, **1–18 kHz** | **Replace scale in JUCE** |
| CROSSOVER | Printed 250–3000 Hz | `exciter_crossover`, **100–5000 Hz** | **Replace scale in JUCE** |
| OUTPUT | −12..+12 dB | `exciter_output_db`, **−12..+12 dB** | Pass |
| MIX | 0–100% | `exciter_mix`, **0–100%** | Pass |
| VU | Needle baked into artwork | No parameter ID or published meter API | Separate physical face and dynamic JUCE needle, then add *read-only* meter telemetry without modifying DSP processing |

## Rendering strategy

Keep ornate brass and teal/cream decorative faceplate as artwork. **All numeric scales, ticks, and changing values must be drawn in code** so the original source's incorrect numbers are never used in a production UI. Rendering should not paste a dynamic knob on top of a baked static knob. The inpainted placeholder is *only* a working mask validation; it is not a finished underlay. Need a texture-faithful knob-free panel with manual cleanup or original layered assets before implementation. 

Rotor references are pixel extracts for texture reference and rotational prototypes. They have not yet been tested for rotating without baked-ring artifacts; rebuild using the same style if necessary. Meter needle must be independent, and never fake its movement from button state.

The controls are geometry-measured references only: final hitboxes and scaling must be checked using actual JUCE editor screenshots.

## Source code verified, no changes

`ThomCofficial/SteamMaster-` → `vst3/v1.7-performance-character` → `plugin/PluginProcessor.cpp` (parameter layout and mapping). Existing APVTS state management is reused; no new parameter IDs are proposed.
'''
(ROOT/'README.md').write_text(readme,encoding='utf8')
shutil.copyfile(__file__, ROOT/'scripts'/'make_exciter_assets.py')

# Self-check all source IDs and IDs distinct per control where they should be.
for c in controls:
 assert c['id'] is None or c['id'] in spec,(c['key'],c['id'])
assert len([x for x in controls if x['type']=='rotor'])==7
assert all(spec[x]['max']==20 for x in ('exciter_low_drive','exciter_high_drive'))
assert len(spec)==9

Z=Path('/mnt/data/SteamMaster_Exciter_Complete_Asset_Prep_v1.zip')
with zipfile.ZipFile(Z,'w',compression=zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for p in sorted(ROOT.rglob('*')):
  if p.is_file():z.write(p,p.relative_to(ROOT.parent))
print('Created:',Z,'files:',len([p for p in ROOT.rglob('*') if p.is_file()]),'size bytes',Z.stat().st_size)
print('QA:',ROOT/'qa'/'Exciter_Whole_Module_QA.png')
print('contact sheet:',ROOT/'qa'/'Exciter_All_Extracted_Control_Refs.png')
