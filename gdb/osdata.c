/* Routines for handling XML generic OS data provided by target.

   Copyright (C) 2008, 2009 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "defs.h"
#include "target.h"
#include "vec.h"
#include "xml-support.h"
#include "osdata.h"
#include "gdb_string.h"
#include "ui-out.h"
#include "gdbcmd.h"

#if !defined(HAVE_LIBEXPAT)

struct osdata *
osdata_parse (const char *xml)
{
  static int have_warned;

  if (!have_warned)
    {
      have_warned = 1;
      warning (_("Can not parse XML OS data; XML support was disabled "
                "at compile time"));
    }

  return NULL;
}

#else /* HAVE_LIBEXPAT */

#include "xml-support.h"

/* Internal parsing data passed to all XML callbacks.  */
struct osdata_parsing_data
  {
    struct osdata *osdata;
    char *property_name;
  };

/* Handle the start of a <osdata> element.  */

static void
osdata_start_osdata (struct gdb_xml_parser *parser,
                        const struct gdb_xml_element *element,
                        void *user_data, VEC(gdb_xml_value_s) *attributes)
{
  struct osdata_parsing_data *data = user_data;
  char *type;
  struct osdata *osdata;

  if (data->osdata)
    gdb_xml_error (parser, _("Seen more than on osdata element"));

  type = VEC_index (gdb_xml_value_s, attributes, 0)->value;
  osdata = XZALLOC (struct osdata);
  osdata->type = xstrdup (type);
  data->osdata = osdata;
}

/* Handle the start of a <item> element.  */

static void
osdata_start_item (struct gdb_xml_parser *parser,
                  const struct gdb_xml_element *element,
                  void *user_data, VEC(gdb_xml_value_s) *attributes)
{
  struct osdata_parsing_data *data = user_data;
  struct osdata_item item = { NULL };
  VEC_safe_push (osdata_item_s, data->osdata->items, &item);
}

/* Handle the start of a <column> element.  */

static void
osdata_start_column (struct gdb_xml_parser *parser,
                    const struct gdb_xml_element *element,
                    void *user_data, VEC(gdb_xml_value_s) *attributes)
{
  struct osdata_parsing_data *data = user_data;
  const char *name = VEC_index (gdb_xml_value_s, attributes, 0)->value;
  data->property_name = xstrdup (name);
}

/* Handle the end of a <column> element.  */

static void
osdata_end_column (struct gdb_xml_parser *parser,
                  const struct gdb_xml_element *element,
                  void *user_data, const char *body_text)
{
  struct osdata_parsing_data *data = user_data;
  struct osdata *osdata = data->osdata;
  struct osdata_item *item = VEC_last (osdata_item_s, osdata->items);
  struct osdata_column *col = VEC_safe_push (osdata_column_s,
                                            item->columns, NULL);

  /* Transfer memory ownership.  NAME was already strdup'ed.  */
  col->name = data->property_name;
  col->value = xstrdup (body_text);
  data->property_name = NULL;
}

/* Discard the constructed osdata (if an error occurs).  */

static void
clear_parsing_data (void *p)
{
  struct osdata_parsing_data *data = p;
  osdata_free (data->osdata);
  data->osdata = NULL;
  xfree (data->property_name);
  data->property_name = NULL;
}

/* The allowed elements and attributes for OS data object.
   The root element is a <osdata>.  */

const struct gdb_xml_attribute column_attributes[] = {
  { "name", GDB_XML_AF_NONE, NULL, NULL },
  { NULL, GDB_XML_AF_NONE, NULL, NULL }
};

const struct gdb_xml_element item_children[] = {
  { "column", column_attributes, NULL,
    GDB_XML_EF_REPEATABLE | GDB_XML_EF_OPTIONAL,
    osdata_start_column, osdata_end_column },
  { NULL, NULL, NULL, GDB_XML_EF_NONE, NULL, NULL }
};

const struct gdb_xml_attribute osdata_attributes[] = {
  { "type", GDB_XML_AF_NONE, NULL, NULL },
  { NULL, GDB_XML_AF_NONE, NULL, NULL }
};

const struct gdb_xml_element osdata_children[] = {
  { "item", NULL, item_children,
    GDB_XML_EF_REPEATABLE | GDB_XML_EF_OPTIONAL,
    osdata_start_item, NULL },
  { NULL, NULL, NULL, GDB_XML_EF_NONE, NULL, NULL }
};

const struct gdb_xml_element osdata_elements[] = {
  { "osdata", osdata_attributes, osdata_children,
    GDB_XML_EF_NONE, osdata_start_osdata, NULL },
  { NULL, NULL, NULL, GDB_XML_EF_NONE, NULL, NULL }
};

struct osdata *
osdata_parse (const char *xml)
{
  struct gdb_xml_parser *parser;
  struct cleanup *before_deleting_result, *back_to;
  struct osdata_parsing_data data = { NULL };

  back_to = make_cleanup (null_cleanup, NULL);
  parser = gdb_xml_create_parser_and_cleanup (_("osdata"),
                                             osdata_elements, &data);
  gdb_xml_use_dtd (parser, "osdata.dtd");

  before_deleting_result = make_cleanup (clear_parsing_data, &data);

  if (gdb_xml_parse (parser, xml) == 0)
    /* Parsed successfully, don't need to delete the result.  */
    discard_cleanups (before_deleting_result);

  do_cleanups (back_to);
  return data.osdata;
}
#endif

static void
osdata_item_clear (struct osdata_item *item)
{
  if (item->columns != NULL)
    {
      struct osdata_column *col;
      int ix;
      for (ix = 0;
	   VEC_iterate (osdata_column_s, item->columns,
			ix, col);
	   ix++)
       {
	 xfree (col->name);
	 xfree (col->value);
       }
      VEC_free (osdata_column_s, item->columns);
      item->columns = NULL;
    }
}

void
osdata_free (struct osdata *osdata)
{
  if (osdata == NULL)
    return;

  if (osdata->items != NULL)
    {
      struct osdata_item *item;
      int ix;
      for (ix = 0;
          VEC_iterate (osdata_item_s, osdata->items,
                       ix, item);
          ix++)
       osdata_item_clear (item);
      VEC_free (osdata_item_s, osdata->items);
    }

  xfree (osdata);
}

static void
osdata_free_cleanup (void *arg)
{
  struct osdata *osdata = arg;
  osdata_free (osdata);
}

struct cleanup *
make_cleanup_osdata_free (struct osdata *data)
{
  return make_cleanup (osdata_free_cleanup, data);
}

struct osdata *
get_osdata (const char *type)
{
  struct osdata *osdata = NULL;
  char *xml = target_get_osdata (type);
  if (xml)
    {
      struct cleanup *old_chain = make_cleanup (xfree, xml);

      if (xml[0] == '\0')
	warning (_("Empty data returned by target.  Wrong osdata type?"));
      else
	osdata = osdata_parse (xml);

      do_cleanups (old_chain);
    }

  if (!osdata)
    error (_("Can not fetch data now.\n"));

  return osdata;
}

const char *
get_osdata_column (struct osdata_item *item, const char *name)
{
  struct osdata_column *col;
  int ix_cols; 
  
  for (ix_cols = 0;
       VEC_iterate (osdata_column_s, item->columns,
		    ix_cols, col);
       ix_cols++)
    if (strcmp (col->name, name) == 0)
      return col->value;

  return NULL;
}

static void
info_osdata_command (char *type, int from_tty)
{
  struct osdata *osdata = NULL;
  struct osdata_item *last;
  struct cleanup *old_chain;
  int ncols;
  int nprocs;

  if (type == 0)
    /* TODO: No type could mean "list availables types".  */
    error (_("Argument required."));

  osdata = get_osdata (type);
  old_chain = make_cleanup_osdata_free (osdata);

  nprocs = VEC_length (osdata_item_s, osdata->items);

  last = VEC_last (osdata_item_s, osdata->items);
  if (last && last->columns)
    ncols = VEC_length (osdata_column_s, last->columns);
  else
    ncols = 0;

  make_cleanup_ui_out_table_begin_end (uiout, ncols, nprocs,
				       "OSDataTable");

  if (last && last->columns)
    {
      struct osdata_column *col;
      int ix;
      for (ix = 0;
          VEC_iterate (osdata_column_s, last->columns,
                       ix, col);
          ix++)
       ui_out_table_header (uiout, 10, ui_left,
                            col->name, col->name);
    }

  ui_out_table_body (uiout);

  if (nprocs != 0)
    {
      struct osdata_item *item;
      int ix_items;
      for (ix_items = 0;
          VEC_iterate (osdata_item_s, osdata->items,
                       ix_items, item);
          ix_items++)
       {
         struct cleanup *old_chain;
         struct ui_stream *stb;
         int ix_cols;
         struct osdata_column *col;

         stb = ui_out_stream_new (uiout);
         old_chain = make_cleanup_ui_out_stream_delete (stb);
         make_cleanup_ui_out_tuple_begin_end (uiout, "item");

         for (ix_cols = 0;
              VEC_iterate (osdata_column_s, item->columns,
                           ix_cols, col);
              ix_cols++)
           ui_out_field_string (uiout, col->name, col->value);

         do_cleanups (old_chain);

         ui_out_text (uiout, "\n");
       }
    }

  do_cleanups (old_chain);
}

extern initialize_file_ftype _initialize_osdata; /* -Wmissing-prototypes */

void
_initialize_osdata (void)
{
  add_info ("os", info_osdata_command,
           _("Show OS data ARG."));
}
