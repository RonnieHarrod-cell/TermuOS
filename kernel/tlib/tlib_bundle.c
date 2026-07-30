#include "tlib_bundle.h"
#include "../mm/heap.h"
#include "../fs/vfs.h"
#include "../lib/printf.h"
#include "../lib/string.h"

// Internal string helpers

static int tl_strlen(const char *s)
{
  int n = 0;
  while (s[n])
    n++;
  return n;
}

// Copy at most max - 1 characters from src into dst and terminate it with a null byte.
static void tl_strncpy(char *dst, const char *src, int max)
{
  int i = 0;
  while (src[i] && i < max - 1)
  {
    dst[i] = src[i];
    i++;
  }
  dst[i] = '\0';
}

// Append src to dst, keeping within the buffer's maximum size.
static void tl_strncat(char *dst, const char *src, int max)
{
  int dlen = tl_strlen(dst);
  int i = 0;
  while (src[i] && dlen + i < max - 1)
  {
    dst[dlen + i] = src[i];
    i++;
  }
  dst[dlen + i] = '\0';
}

static int tl_strncmp(const char *a, const char *b, int n)
{
  for (int i = 0; i < n; i++)
  {
    if (a[i] != b[i])
      return (unsigned char)a[i] - (unsigned char)b[i];
    if (!a[i])
      return 0;
  }
  return 0;
}

static int tl_isspace(char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// Minimal JSON parser
// This only handles the flat structure used by tapp manifests:
//   - string values: "key": "value"
//   - string arrays: "key": ["a", "b"]
// There is no nesting beyond one array level, and no numbers or booleans.

typedef struct
{
  const char *src;
  int pos;
  int len;
} json_ctx_t;

static void json_skip_ws(json_ctx_t *j)
{
  while (j->pos < j->len && tl_isspace(j->src[j->pos]))
    j->pos++;
}

static char json_peek(json_ctx_t *j)
{
  json_skip_ws(j);
  if (j->pos >= j->len)
    return '\0';
  return j->src[j->pos];
}

static char json_consume(json_ctx_t *j)
{
  json_skip_ws(j);
  if (j->pos >= j->len)
    return '\0';
  return j->src[j->pos++];
}

// Parse a JSON string. The opening quote is assumed to be already consumed.
// The function writes up to max - 1 characters into buf and returns 0 on success.
static int json_parse_string(json_ctx_t *j, char *buf, int max)
{
  if (json_consume(j) != '"')
    return -1;
  int i = 0;
  while (j->pos < j->len)
  {
    char c = j->src[j->pos++];
    if (c == '"')
    {
      buf[i] = '\0';
      return 0;
    }
    if (c == '\\' && j->pos < j->len)
      c = j->src[j->pos++]; // skip escape
    if (i < max - 1)
      buf[i++] = c;
  }
  return -1; // unterminated string
}

// Parse a JSON string array such as ["a","b",...] into out[][col_max].
// The count of parsed elements is written to *count.
static int json_parse_str_array(json_ctx_t *j, char (*out)[TLIB_PERM_LEN],
                                int col_max, int row_max, int *count)
{
  *count = 0;
  if (json_consume(j) != '[')
    return -1;
  while (1)
  {
    char p = json_peek(j);
    if (p == ']')
    {
      j->pos++;
      return 0;
    }
    if (p == ',')
    {
      j->pos++;
      continue;
    }
    if (p == '"')
    {
      if (*count >= row_max)
        return -1;
      if (json_parse_string(j, out[*count], col_max) != 0)
        return -1;
      (*count)++;
    }
    else
      return -1;
  }
}

// Permission string to bit-flag mapping

static uint32_t perm_to_flag(const char *perm)
{
  if (tl_strncmp(perm, "ipc.send", 8) == 0)
    return TLIB_PERM_IPC_SEND;
  if (tl_strncmp(perm, "ipc.receive", 11) == 0)
    return TLIB_PERM_IPC_RECEIVE;
  if (tl_strncmp(perm, "fs.read", 7) == 0)
    return TLIB_PERM_FS_READ;
  if (tl_strncmp(perm, "fs.write", 8) == 0)
    return TLIB_PERM_FS_WRITE;
  if (tl_strncmp(perm, "proc.spawn", 10) == 0)
    return TLIB_PERM_PROC_SPAWN;
  kprintf("tlib: unknown permission '%s' (ignored)\n", perm);
  return 0;
}

// Manifest parsing

int tlib_manifest_parse(const char *json, tlib_manifest_t *out)
{
  if (!json || !out)
    return -1;

  // Clear the manifest structure.
  for (size_t i = 0; i < sizeof(*out); i++)
    ((char *)out)[i] = 0;

  json_ctx_t j = {.src = json, .pos = 0, .len = tl_strlen(json)};

  // Expect the opening brace.
  if (json_consume(&j) != '{')
    return -1;

  while (1)
  {
    char p = json_peek(&j);
    if (p == '}')
      break;
    if (p == ',')
    {
      j.pos++;
      continue;
    }
    if (p != '"')
      return -1;

    // Parse the key.
    char key[64];
    if (json_parse_string(&j, key, sizeof(key)) != 0)
      return -1;

    // Expect a colon.
    if (json_consume(&j) != ':')
      return -1;

    // Dispatch based on the key.
    if (tl_strncmp(key, "name", 4) == 0 && tl_strlen(key) == 4)
    {
      if (json_parse_string(&j, out->name, TLIB_NAME_MAX) != 0)
        return -1;
    }
    else if (tl_strncmp(key, "bundle_id", 9) == 0 && tl_strlen(key) == 9)
    {
      if (json_parse_string(&j, out->bundle_id, TLIB_BUNDLE_ID_MAX) != 0)
        return -1;
    }
    else if (tl_strncmp(key, "version", 7) == 0 && tl_strlen(key) == 7)
    {
      if (json_parse_string(&j, out->version, TLIB_VERSION_MAX) != 0)
        return -1;
    }
    else if (tl_strncmp(key, "entry", 5) == 0 && tl_strlen(key) == 5)
    {
      if (json_parse_string(&j, out->entry, TLIB_ENTRY_MAX) != 0)
        return -1;
    }
    else if (tl_strncmp(key, "permissions", 11) == 0 && tl_strlen(key) == 11)
    {
      // The permissions array uses the same element width as ports, so the cast is safe.
      if (json_parse_str_array(&j, out->permissions,
                               TLIB_PERM_LEN, TLIB_MAX_PERMS,
                               &out->perm_count) != 0)
        return -1;
    }
    else if (tl_strncmp(key, "ports", 5) == 0 && tl_strlen(key) == 5)
    {
      // Ports are stored as TLIB_PORT_LEN-wide entries, so we cast carefully here.
      if (json_parse_str_array(&j, (char (*)[TLIB_PERM_LEN])out->ports,
                               TLIB_PORT_LEN, TLIB_MAX_PORTS,
                               &out->port_count) != 0)
        return -1;
    }
    else
    {
      // This is an unknown key, so skip its value whether it is a string or an array.
      char p2 = json_peek(&j);
      if (p2 == '"')
      {
        char tmp[256];
        json_parse_string(&j, tmp, sizeof(tmp));
      }
      else if (p2 == '[')
      {
        // consume until matching ']'
        int depth = 0;
        while (j.pos < j.len)
        {
          char c = j.src[j.pos++];
          if (c == '[')
            depth++;
          else if (c == ']')
          {
            depth--;
            if (!depth)
              break;
          }
        }
      }
    }
  }

  // Validate the required fields.
  if (!out->name[0] || !out->bundle_id[0] || !out->entry[0])
  {
    kprintf("tlib: manifest missing required field (name/bundle_id/entry)\n");
    return -1;
  }

  // build perm_mask
  out->perm_mask = 0;
  for (int i = 0; i < out->perm_count; i++)
    out->perm_mask |= perm_to_flag(out->permissions[i]);

  return 0;
}

// ── tlib_bundle_load ──────────────────────────────────────────────────────────

int tlib_bundle_load(const char *bundle_path, tlib_app_t *out)
{
  if (!bundle_path || !out)
    return -1;

  // zero output
  for (size_t i = 0; i < sizeof(*out); i++)
    ((char *)out)[i] = 0;

  tl_strncpy(out->bundle_path, bundle_path, sizeof(out->bundle_path));

  // ── 1. stat the bundle directory ─────────────────────────────────────────
  uint32_t type;
  uint64_t size;
  if (vfs_stat(bundle_path, &type, &size) < 0)
  {
    kprintf("tlib: bundle not found: %s\n", bundle_path);
    return -1;
  }
  if (type != VFS_DIR)
  {
    kprintf("tlib: bundle path is not a directory: %s\n", bundle_path);
    return -1;
  }

  // ── 2. build path to manifest.json ───────────────────────────────────────
  char manifest_path[300];
  tl_strncpy(manifest_path, bundle_path, sizeof(manifest_path));
  tl_strncat(manifest_path, "/manifest.json", sizeof(manifest_path));

  // ── 3. read manifest.json via VFS ────────────────────────────────────────
  uint32_t mtype;
  uint64_t msize;
  if (vfs_stat(manifest_path, &mtype, &msize) < 0 || mtype != VFS_FILE)
  {
    kprintf("tlib: manifest.json not found in %s\n", bundle_path);
    return -1;
  }
  if (msize == 0 || msize > 4096)
  {
    kprintf("tlib: manifest.json has invalid size (%u bytes)\n",
            (uint32_t)msize);
    return -1;
  }

  char *buf = (char *)kmalloc((size_t)msize + 1);
  if (!buf)
  {
    kprintf("tlib: OOM reading manifest\n");
    return -1;
  }

  int fd = vfs_open(manifest_path, O_RDONLY);
  if (fd < 0)
  {
    kfree(buf);
    kprintf("tlib: could not open %s\n", manifest_path);
    return -1;
  }

  int nread = vfs_read(fd, buf, (size_t)msize);
  vfs_close(fd);

  if (nread <= 0)
  {
    kfree(buf);
    kprintf("tlib: failed to read manifest.json\n");
    return -1;
  }
  buf[nread] = '\0';

  // ── 4. parse manifest ─────────────────────────────────────────────────────
  if (tlib_manifest_parse(buf, &out->manifest) != 0)
  {
    kfree(buf);
    kprintf("tlib: failed to parse manifest in %s\n", bundle_path);
    return -1;
  }
  kfree(buf);

  // ── 5. resolve entry path ─────────────────────────────────────────────────
  tl_strncpy(out->entry_path, bundle_path, sizeof(out->entry_path));
  tl_strncat(out->entry_path, "/", sizeof(out->entry_path));
  tl_strncat(out->entry_path, out->manifest.entry, sizeof(out->entry_path));

  // confirm the entry binary exists
  uint32_t etype;
  uint64_t esize;
  if (vfs_stat(out->entry_path, &etype, &esize) < 0 || etype != VFS_FILE)
  {
    kprintf("tlib: entry binary not found: %s\n", out->entry_path);
    return -1;
  }

  out->loaded = 1;
  kprintf("tlib: loaded '%s' (%s) from %s\n",
          out->manifest.name, out->manifest.bundle_id, bundle_path);
  return 0;
}

// ── tlib_manifest_dump ────────────────────────────────────────────────────────

void tlib_manifest_dump(const tlib_manifest_t *m)
{
  kprintf("─── tlib manifest ───────────────────\n");
  kprintf("  name:       %s\n", m->name);
  kprintf("  bundle_id:  %s\n", m->bundle_id);
  kprintf("  version:    %s\n", m->version);
  kprintf("  entry:      %s\n", m->entry);
  kprintf("  perm_mask:  0x%x\n", m->perm_mask);
  kprintf("  perms (%d):\n", m->perm_count);
  for (int i = 0; i < m->perm_count; i++)
    kprintf("    - %s\n", m->permissions[i]);
  kprintf("  ports (%d):\n", m->port_count);
  for (int i = 0; i < m->port_count; i++)
    kprintf("    - %s\n", m->ports[i]);
  kprintf("─────────────────────────────────────\n");
}

static uint32_t current_perm_mask = 0xffffffff; // kernel: all perms

void tlib_set_perm_mask(uint32_t mask)
{
  current_perm_mask = mask;
}

int tlib_check_perm(uint32_t perm)
{
  if (current_perm_mask == 0xffffffff)
    return 1; // kernel process
  if (current_perm_mask & perm)
    return 1;
  kprintf("tlib: permission denied (needed 0x%x, have 0x%x)\n", perm, current_perm_mask);
  return 0;
}
