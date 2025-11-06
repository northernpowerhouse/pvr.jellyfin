#!/usr/bin/env python3
"""
Script to update the Kodi addon repository with new builds.
This script generates addons.xml and addons.xml.md5 files.
"""

import os
import hashlib
import xml.etree.ElementTree as ET
from pathlib import Path
import zipfile
import shutil

def get_addon_info_from_zip(zip_path):
    """Extract addon.xml from zip and parse it."""
    try:
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            # Look for addon.xml in the root or in a subdirectory
            addon_xml_path = None
            for name in zip_ref.namelist():
                if name.endswith('addon.xml'):
                    addon_xml_path = name
                    break
            
            if not addon_xml_path:
                return None
            
            with zip_ref.open(addon_xml_path) as f:
                tree = ET.parse(f)
                return tree.getroot()
    except Exception as e:
        print(f"Error reading {zip_path}: {e}")
        return None

def generate_addons_xml(repo_dir):
    """Generate addons.xml file from all addon zips in repository."""
    addons_xml = ET.Element('addons')
    
    # Find all addon zips
    for zip_file in Path(repo_dir).rglob('*.zip'):
        if 'repository.' in str(zip_file):
            continue  # Skip repository addon itself
        
        addon_info = get_addon_info_from_zip(zip_file)
        if addon_info is not None:
            # Add download URL info
            relative_path = zip_file.relative_to(repo_dir)
            # Note: In a real scenario, you'd set the actual URL
            # For now, we'll use a relative path
            addons_xml.append(addon_info)
    
    # Write addons.xml
    tree = ET.ElementTree(addons_xml)
    ET.indent(tree, space='  ')
    addons_xml_path = Path(repo_dir) / 'addons.xml'
    tree.write(addons_xml_path, encoding='UTF-8', xml_declaration=True)
    
    return addons_xml_path

def generate_md5(file_path):
    """Generate MD5 hash for a file."""
    md5_hash = hashlib.md5()
    with open(file_path, 'rb') as f:
        for chunk in iter(lambda: f.read(4096), b''):
            md5_hash.update(chunk)
    return md5_hash.hexdigest()

def update_repository():
    """Main function to update the repository."""
    # The script is run from within the addon-repo directory
    # So current working directory IS the repository
    repo_dir = Path.cwd()
    
    print(f"Updating repository in {repo_dir}")
    
    # Check if pvr.jellyfin directory exists with zips
    pvr_jellyfin_dir = repo_dir / 'pvr.jellyfin'
    if not pvr_jellyfin_dir.exists():
        print("Error: pvr.jellyfin directory not found!")
        return
    
    zip_files = list(pvr_jellyfin_dir.glob('*.zip'))
    if not zip_files:
        print("Warning: No zip files found in pvr.jellyfin directory!")
        return
    
    print(f"Found {len(zip_files)} addon zip file(s):")
    for zf in zip_files:
        print(f"  - {zf.name}")
    
    # Generate addons.xml
    print("\nGenerating addons.xml...")
    addons_xml_path = generate_addons_xml(repo_dir)
    
    # Generate addons.xml.md5
    print("Generating addons.xml.md5...")
    md5_value = generate_md5(addons_xml_path)
    md5_path = Path(str(addons_xml_path) + '.md5')
    with open(md5_path, 'w') as f:
        f.write(md5_value)
    
    print("Repository updated successfully!")

if __name__ == '__main__':
    update_repository()
