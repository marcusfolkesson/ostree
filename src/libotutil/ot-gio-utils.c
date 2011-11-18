/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*-
 *
 * Copyright (C) 2011 Colin Walters <walters@verbum.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Colin Walters <walters@verbum.org>
 */

#include "config.h"

#include <gio/gio.h>
#include <gio/gunixinputstream.h>

#include <string.h>

#include "otutil.h"

gboolean
ot_gfile_ensure_directory (GFile     *dir,
                           gboolean   with_parents, 
                           GError   **error)
{
  GError *temp_error = NULL;
  gboolean ret = FALSE;

  if (with_parents)
    ret = g_file_make_directory_with_parents (dir, NULL, &temp_error);
  else
    ret = g_file_make_directory (dir, NULL, &temp_error);
  if (!ret)
    {
      if (!g_error_matches (temp_error, G_IO_ERROR, G_IO_ERROR_EXISTS))
        {
          g_propagate_error (error, temp_error);
          goto out;
        }
      else
        g_clear_error (&temp_error);
    }

  ret = TRUE;
 out:
  return ret;
}

gboolean
ot_gfile_load_contents_utf8 (GFile         *file,
                             char         **contents_out,
                             char         **etag_out,
                             GCancellable  *cancellable,
                             GError       **error)
{
  char *ret_contents = NULL;
  char *ret_etag = NULL;
  gsize len;
  gboolean ret = FALSE;

  if (!g_file_load_contents (file, cancellable, &ret_contents, &len, &ret_etag, error))
    goto out;
  if (!g_utf8_validate (ret_contents, len, NULL))
    {
      g_set_error (error,
                   G_IO_ERROR,
                   G_IO_ERROR_INVALID_DATA,
                   "Invalid UTF-8");
      goto out;
    }

  if (contents_out)
    *contents_out = ret_contents;
  else
    g_free (ret_contents);
  ret_contents = NULL;
  if (etag_out)
    *etag_out = ret_etag;
  else
    g_free (ret_etag);
  ret_etag = NULL;
  ret = TRUE;
 out:
  g_free (ret_contents);
  g_free (ret_etag);
  return ret;
}

/* Like g_file_new_for_path, but only do local stuff, not GVFS */
GFile *
ot_gfile_new_for_path (const char *path)
{
  return g_vfs_get_file_for_path (g_vfs_get_local (), path);
}

const char *
ot_gfile_get_path_cached (GFile *file)
{
  const char *path;

  path = g_object_get_data ((GObject*)file, "ostree-file-path");
  if (!path)
    {
      path = g_file_get_path (file);
      g_object_set_data_full ((GObject*)file, "ostree-file-path", (char*)path, (GDestroyNotify)g_free);
    }
  return path;
}


const char *
ot_gfile_get_basename_cached (GFile *file)
{
  const char *name;

  name = g_object_get_data ((GObject*)file, "ostree-file-name");
  if (!name)
    {
      name = g_file_get_basename (file);
      g_object_set_data_full ((GObject*)file, "ostree-file-name", (char*)name, (GDestroyNotify)g_free);
    }
  return name;
}
