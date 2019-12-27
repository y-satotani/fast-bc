#!/usr/bin/env python3

import sys
import requests

def download_file(file_id, file_name):
    url = f'https://drive.google.com/uc'
    payload = {'export': 'download', 'id': file_id}
    r = requests.get(url, params=payload, stream=True)
    try:
        cookies = r.cookies
        code = cookies[next(
            k for k in r.cookies.keys() if k.startswith('download_warning_')
        )]
        payload['confirm'] = code
        r = requests.get(url, params=payload, stream=True, cookies=cookies)
    except StopIteration:
        pass
    with open(file_name, 'wb') as fd:
        for chunk in r.iter_content(chunk_size=1024):
            fd.write(chunk)

if __name__ == '__main__':
    try:
        with open('content_list', 'r') as fp:
            arguments = [l.split() for l in fp.read().splitlines()]
    except RuntimeError as e:
        print(e, file=sys.stderr)
        exit(1)
    for file_id, file_name in arguments:
        print(f'downloading {file_name}')
        download_file(file_id, file_name)
