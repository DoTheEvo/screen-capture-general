#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "screenshot-utils.h"
#include <X11/Xatom.h>
#include <gdk/gdkx.h>

static gboolean
get_atom_property (Window  xwindow,
		   Atom    atom,
		   Atom   *val)
{
  Atom type;
  int format;
  gulong nitems;
  gulong bytes_after;
  Atom *a = NULL;
  int err, result;
  guchar *atom_cast;

  *val = 0;
  
  gdk_error_trap_push ();
  type = None;
  result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
			       xwindow,
			       atom,
			       0, G_MAXLONG,
			       False, XA_ATOM, &type, &format, &nitems,
			       &bytes_after, &atom_cast);  
  a = (Atom *) atom_cast;
  err = gdk_error_trap_pop ();
  if (err != Success ||
      result != Success)
    return FALSE;
  
  if (type != XA_ATOM)
    {
      XFree (a);
      return FALSE;
    }

  *val = *a;
  
  XFree (a);

  return TRUE;
}

static Window
find_toplevel_window (Window xid)
{
  Window root, parent, *children;
  guint nchildren;

  do
    {
      if (XQueryTree (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), xid, &root,
		      &parent, &children, &nchildren) == 0)
	{
	  g_warning ("Couldn't find window manager window");
	  return None;
	}

      if (root == parent)
	return xid;

      xid = parent;
    }
  while (TRUE);
}

static gboolean
screenshot_window_is_desktop (Window xid)
{
  Window root_window = GDK_ROOT_WINDOW ();

  if (xid == root_window)
    return TRUE;

  if (gdk_x11_screen_supports_net_wm_hint (gdk_screen_get_default (),
                                           gdk_atom_intern ("_NET_WM_WINDOW_TYPE", FALSE)))
    {
      gboolean retval;
      Atom property;

      retval = get_atom_property (xid,
				  gdk_x11_get_xatom_by_name ("_NET_WM_WINDOW_TYPE"),
				  &property);
      if (retval &&
	  property == gdk_x11_get_xatom_by_name ("_NET_WM_WINDOW_TYPE_DESKTOP"))
	return TRUE;
    }
  return FALSE;
}

/* We don't actually honor include_decoration here.  We need to search
 * for WM_STATE;
 */
static Window
screenshot_find_pointer_window (void)
{
  Window root_window, root_return, child;
  int unused;
  guint mask;

  root_window = GDK_ROOT_WINDOW ();

  XQueryPointer (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), root_window,
		 &root_return, &child, &unused,
		 &unused, &unused, &unused, &mask);

  return child;
}

#define MAXIMUM_WM_REPARENTING_DEPTH 4

/* adopted from eel code */
static Window
look_for_hint_helper (Window    xid,
		      Atom      property,
		      int       depth)
{
  Atom actual_type;
  int actual_format;
  gulong nitems, bytes_after;
  gulong *prop;
  Window root, parent, *children, window;
  guint nchildren, i;

  if (XGetWindowProperty (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
                          xid, property, 0, 1,
			  False, AnyPropertyType, &actual_type,
			  &actual_format, &nitems, &bytes_after,
			  (gpointer) &prop) == Success
      && prop != NULL && actual_format == 32 && prop[0] == NormalState)
    {
      if (prop != NULL)
	{
	  XFree (prop);
	}

      return xid;
    }

  if (depth < MAXIMUM_WM_REPARENTING_DEPTH)
    {
      if (XQueryTree (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), xid, &root,
		      &parent, &children, &nchildren) != 0)
	{
	  window = None;

	  for (i = 0; i < nchildren; i++)
	    {
	      window = look_for_hint_helper (children[i],
					     property,
					     depth + 1);
	      if (window != None)
		break;
	    }

	  if (children != NULL)
	    XFree (children);

	  if (window)
	    return window;
	}
    }

  return None;
}

static Window
look_for_hint (Window xid,
	       Atom property)
{
  Window retval;

  retval = look_for_hint_helper (xid, property, 0);

  return retval;
}


Window
screenshot_find_current_window (gboolean include_decoration)
{
  Window current_window;

  current_window = screenshot_find_pointer_window ();

  if (current_window)
    {
      if (screenshot_window_is_desktop (current_window))
	/* if the current window is the desktop (eg. nautilus), we
	 * return None, as getting the whole screen makes more sense. */
	return None;

      /* Once we have a window, we walk the widget tree looking for
       * the appropriate window. */
      current_window = find_toplevel_window (current_window);
      if (! include_decoration)
	{
	  Window new_window;
	  new_window = look_for_hint (current_window, gdk_x11_get_xatom_by_name ("WM_STATE"));
	  if (new_window)
	    current_window = new_window;
	}
    }
  return current_window;
}

