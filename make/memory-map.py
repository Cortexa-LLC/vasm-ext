#!/usr/bin/env python3
"""
Memory Map Generator for vasm builds

Parses vasm listing files to extract ORG addresses and sizes,
then generates a memory map showing file layout and runtime addresses.

Usage:
    memory-map.py [options] <listing_file> ...
    memory-map.py --config <config_file>

Options:
    -o, --output FILE       Output file (default: stdout)
    -c, --config FILE       JSON config file describing build layout
    --max-symbols N         Max symbols per listing file (0=all, default=50)
    --no-symbols            Don't include symbol table
"""

import argparse
import json
import os
import re
import sys
from datetime import datetime
from pathlib import Path


def parse_listing_org(listing_path):
    """Parse vasm listing file for ORG address and size.

    Looks for section definitions in vasm listing files:
      00: "org0001:2000" (2000-4AB2)

    Returns the first org section's start address and size.
    """
    # Pattern matches: 00: "org0001:XXXX" (XXXX-YYYY)
    section_pattern = re.compile(
        r'^\d+:\s*"org\d+:([0-9a-fA-F]+)"\s*\(([0-9a-fA-F]+)-([0-9a-fA-F]+)\)'
    )

    with open(listing_path, 'r') as f:
        for line in f:
            match = section_pattern.match(line)
            if match:
                org_addr = int(match.group(1), 16)
                start_addr = int(match.group(2), 16)
                end_addr = int(match.group(3), 16)
                size = end_addr - start_addr + 1
                return org_addr, size

    return None, None


def parse_listing_symbols(listing_path, max_symbols=50):
    """Extract symbols from vasm listing file.

    Looks for lines like: SYMBOL_NAME                    A:XXXX
    """
    symbol_pattern = re.compile(r'^([A-Z][A-Z0-9._]*)\s+A:([0-9A-Fa-f]+)')
    symbols = []

    with open(listing_path, 'r') as f:
        for line in f:
            match = symbol_pattern.match(line)
            if match:
                symbols.append((match.group(1), int(match.group(2), 16)))
                if max_symbols > 0 and len(symbols) >= max_symbols:
                    break

    return symbols


def get_file_size(file_path):
    """Get actual file size in bytes."""
    try:
        return os.path.getsize(file_path)
    except OSError:
        return None


def format_hex(value):
    """Format value as hex with $ prefix."""
    return f"${value:04X}"


def format_size(size):
    """Format size in human-readable form."""
    if size >= 1024:
        return f"~{size // 1024}K"
    return str(size)


def parse_int(value):
    """Parse an integer from string (supports 0x hex) or int."""
    if value is None:
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        value = value.strip()
        if value.startswith('0x') or value.startswith('0X'):
            return int(value, 16)
        if value.startswith('$'):
            return int(value[1:], 16)
        return int(value)
    return None


def generate_memory_map(config, include_symbols=True, max_symbols=50):
    """Generate memory map from config.

    Config format:
    {
        "title": "Project Name",
        "load_address": "0x2000",
        "components": [
            {
                "name": "Component Name",
                "binary": "path/to/binary",
                "listing": "path/to/listing.lst",
                "run_address": "0x8000",  # optional, defaults to ORG from listing
                "pad_to": "0x4B00",       # optional, pad file offset to this
                "size_override": 512      # optional, use this size instead of computed
            },
            ...
        ],
        "runtime_regions": [              # optional, additional runtime memory regions
            {
                "name": "Region Name",
                "start": "0x0800",
                "end": "0x7FFF"
            }
        ]
    }
    """
    output = []

    title = config.get('title', 'Memory Map')
    load_address = parse_int(config.get('load_address', 0))

    output.append(f"=== {title} ===")
    output.append(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    output.append("")

    # Collect component info
    components = []
    file_offset = 0

    for comp in config.get('components', []):
        name = comp.get('name', 'Unknown')
        binary_path = comp.get('binary')
        listing_path = comp.get('listing')

        # Get ORG and size from listing if available
        org_addr = None
        listing_size = None
        if listing_path and os.path.exists(listing_path):
            org_addr, listing_size = parse_listing_org(listing_path)

        # Get actual binary size
        binary_size = None
        if binary_path and os.path.exists(binary_path):
            binary_size = get_file_size(binary_path)

        # Use explicit run_address or fall back to ORG
        run_address = parse_int(comp.get('run_address')) or org_addr

        # Handle padding - pad_to is absolute load address
        pad_to = parse_int(comp.get('pad_to'))
        if pad_to is not None:
            file_offset = pad_to - load_address

        # Size priority: size_override > binary_size > listing_size
        size_override = comp.get('size_override')
        if size_override is not None:
            size = size_override
        else:
            size = binary_size or listing_size or 0

        components.append({
            'name': name,
            'file_offset': file_offset,
            'load_address': load_address + file_offset,
            'run_address': run_address,
            'size': size,
            'listing_path': listing_path
        })

        # Advance file offset (unless next component has explicit pad_to)
        file_offset += size

    # Output file layout
    output.append("=== File Layout ===")
    output.append(f"{'File offset':<12} {'Load addr':<10} {'Run addr':<10} {'Size':<10} {'Component'}")
    output.append(f"{'-'*11:<12} {'-'*9:<10} {'-'*9:<10} {'-'*8:<10} {'-'*9}")

    for comp in components:
        file_off = format_hex(comp['file_offset'])
        load_addr = format_hex(comp['load_address'])
        run_addr = format_hex(comp['run_address']) if comp['run_address'] else "N/A"
        size = format_size(comp['size'])
        output.append(f"{file_off:<12} {load_addr:<10} {run_addr:<10} {size:<10} {comp['name']}")

    output.append("")

    # Build runtime layout from components + additional regions
    runtime_items = []

    for comp in components:
        if comp['run_address'] is not None:
            runtime_items.append({
                'name': comp['name'],
                'start': comp['run_address'],
                'end': comp['run_address'] + comp['size'] - 1,
                'size': comp['size']
            })

    # Add additional runtime regions from config
    for region in config.get('runtime_regions', []):
        start = parse_int(region.get('start'))
        end = parse_int(region.get('end'))
        if start is not None and end is not None:
            runtime_items.append({
                'name': region.get('name', 'Unknown'),
                'start': start,
                'end': end,
                'size': end - start + 1
            })

    # Sort by start address
    runtime_items.sort(key=lambda r: r['start'])

    if runtime_items:
        output.append("=== Runtime Memory Layout ===")
        output.append(f"{'Address range':<16} {'Size':<10} {'Component'}")
        output.append(f"{'-'*14:<16} {'-'*8:<10} {'-'*9}")

        for item in runtime_items:
            addr_range = f"{format_hex(item['start'])}-{format_hex(item['end'])}"
            size = format_size(item['size'])
            output.append(f"{addr_range:<16} {size:<10} {item['name']}")

        output.append("")

    # Output symbols
    if include_symbols:
        output.append("=== Symbols ===")

        for comp in components:
            listing_path = comp.get('listing_path')
            if listing_path and os.path.exists(listing_path):
                output.append(f"--- From {listing_path} ---")
                symbols = parse_listing_symbols(listing_path, max_symbols)
                if symbols:
                    for name, addr in symbols:
                        output.append(f"{name:<40} A:{addr:04X}")
                else:
                    output.append("(no symbols)")
                output.append("")

    return '\n'.join(output)


def main():
    parser = argparse.ArgumentParser(description='Generate memory map from vasm listings')
    parser.add_argument('-c', '--config', help='JSON config file')
    parser.add_argument('-o', '--output', help='Output file (default: stdout)')
    parser.add_argument('--max-symbols', type=int, default=50,
                        help='Max symbols per listing (0=all)')
    parser.add_argument('--no-symbols', action='store_true',
                        help="Don't include symbol table")
    parser.add_argument('listings', nargs='*', help='Listing files (if no config)')

    args = parser.parse_args()

    if args.config:
        with open(args.config, 'r') as f:
            config = json.load(f)
    elif args.listings:
        # Simple mode: just parse listings without layout info
        config = {
            'title': 'Memory Map',
            'components': [
                {'name': Path(lst).stem, 'listing': lst}
                for lst in args.listings
            ]
        }
    else:
        parser.print_help()
        sys.exit(1)

    result = generate_memory_map(
        config,
        include_symbols=not args.no_symbols,
        max_symbols=args.max_symbols
    )

    if args.output:
        with open(args.output, 'w') as f:
            f.write(result)
        print(f"Memory map saved to: {args.output}", file=sys.stderr)
    else:
        print(result)


if __name__ == '__main__':
    main()
