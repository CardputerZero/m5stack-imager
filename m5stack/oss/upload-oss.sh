#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$script_dir"

env_file="${ENV_FILE:-$script_dir/.env}"
if [[ -f "$env_file" ]]; then
  set -a
  # shellcheck disable=SC1090
  source "$env_file"
  set +a
fi

OSS_BUCKET="${OSS_BUCKET:-cardputer-zero-repo}"
OSS_ENDPOINT="${OSS_ENDPOINT:-oss-cn-shenzhen.aliyuncs.com}"
OSS_REGION="${OSS_REGION:-cn-shenzhen}"
PUBLIC_BASE_URL="${PUBLIC_BASE_URL:-https://${OSS_BUCKET}.${OSS_ENDPOINT}}"
OSSUTIL_BIN="${OSSUTIL_BIN:-$(command -v ossutil || command -v ossutil64 || true)}"

if [[ -z "${OSS_ACCESS_KEY_ID:-}" || -z "${OSS_ACCESS_KEY_SECRET:-}" ]]; then
  echo "ERROR: missing OSS_ACCESS_KEY_ID or OSS_ACCESS_KEY_SECRET." >&2
  echo "Create $env_file from .env.example and fill in the credentials." >&2
  exit 1
fi

if [[ ! -f os-list.json ]]; then
  echo "ERROR: os-list.json not found in $script_dir" >&2
  exit 1
fi

python3 -m json.tool os-list.json >/dev/null

# This script only publishes the repository manifest and small static assets.
# OS image artifacts are produced and uploaded by the image release owner.

tmp_config="$(mktemp)"
chmod 600 "$tmp_config"
trap 'rm -f "$tmp_config"' EXIT

cat >"$tmp_config" <<EOF
[default]
mode=AK
accessKeyID=$OSS_ACCESS_KEY_ID
accessKeySecret=$OSS_ACCESS_KEY_SECRET
access-key-id=$OSS_ACCESS_KEY_ID
access-key-secret=$OSS_ACCESS_KEY_SECRET
region=$OSS_REGION
endpoint=$OSS_ENDPOINT
EOF

ossutil_args=(-c "$tmp_config")
if [[ "${DRY_RUN:-0}" == "1" ]]; then
  ossutil_args+=(-n)
fi

upload_file() {
  local source_path="$1"
  local object_key="$2"

  echo "Uploading $source_path -> oss://${OSS_BUCKET}/${object_key}"
  if [[ -n "$OSSUTIL_BIN" ]]; then
    "$OSSUTIL_BIN" "${ossutil_args[@]}" cp -f "$source_path" "oss://${OSS_BUCKET}/${object_key}"
  else
    SOURCE_PATH="$source_path" OBJECT_KEY="$object_key" python3 - <<'PY'
import base64
import email.utils
import hashlib
import hmac
import mimetypes
import os
import sys
import urllib.error
import urllib.request

source_path = os.environ["SOURCE_PATH"]
object_key = os.environ["OBJECT_KEY"].lstrip("/")
bucket = os.environ["OSS_BUCKET"]
endpoint = os.environ["OSS_ENDPOINT"]
access_key_id = os.environ["OSS_ACCESS_KEY_ID"]
access_key_secret = os.environ["OSS_ACCESS_KEY_SECRET"]

content_type = mimetypes.guess_type(source_path)[0] or "application/octet-stream"
content_disposition = "inline" if content_type.startswith("image/") else "inline"
with open(source_path, "rb") as f:
    body = f.read()

content_md5 = base64.b64encode(hashlib.md5(body).digest()).decode("ascii")
date = email.utils.formatdate(usegmt=True)
canonical_resource = f"/{bucket}/{object_key}"
canonical_oss_headers = "x-oss-object-acl:public-read\n"
string_to_sign = "\n".join([
    "PUT",
    content_md5,
    content_type,
    date,
    canonical_oss_headers + canonical_resource,
])
signature = base64.b64encode(
    hmac.new(access_key_secret.encode("utf-8"), string_to_sign.encode("utf-8"), hashlib.sha1).digest()
).decode("ascii")

url = f"https://{bucket}.{endpoint}/{object_key}"
request = urllib.request.Request(
    url,
    data=body,
    method="PUT",
    headers={
        "Authorization": f"OSS {access_key_id}:{signature}",
        "Content-MD5": content_md5,
        "Content-Type": content_type,
        "Content-Disposition": content_disposition,
        "Date": date,
        "Content-Length": str(len(body)),
        "x-oss-object-acl": "public-read",
    },
)

try:
    with urllib.request.urlopen(request, timeout=60) as response:
        if response.status not in (200, 201):
            print(f"ERROR: upload failed with HTTP {response.status}", file=sys.stderr)
            sys.exit(1)
except urllib.error.HTTPError as exc:
    details = exc.read().decode("utf-8", errors="replace")
    print(f"ERROR: upload failed with HTTP {exc.code}: {details}", file=sys.stderr)
    sys.exit(1)
PY
  fi
}

upload_file os-list.json os-list.json

shopt -s nullglob
icon_files=(icons/*.png)
if (( ${#icon_files[@]} == 0 )); then
  echo "No PNG icons found under $script_dir/icons; skipping icon upload."
else
  for icon_file in "${icon_files[@]}"; do
    upload_file "$icon_file" "icons/$(basename "$icon_file")"
  done
fi

echo
echo "Published URLs:"
echo "  ${PUBLIC_BASE_URL}/os-list.json"
for icon_file in "${icon_files[@]:-}"; do
  echo "  ${PUBLIC_BASE_URL}/icons/$(basename "$icon_file")"
done
