/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

/* GdkImageDB: hacked up version of gtkimage.c/gtkimage.h from gtk+-1.2.10 */

#include <gtk/gtkcontainer.h>
#include <gdk/gdk.h>
#include "gtkimagedb.h"


static void gtk_imagedb_class_init (GtkImageDBClass  *klass);
static void gtk_imagedb_init       (GtkImageDB       *image);
static gint gtk_imagedb_expose     (GtkWidget      *widget,
				  GdkEventExpose *event);


GtkType
gtk_imagedb_get_type (void)
{
  static GtkType image_type = 0;

  if (!image_type)
    {
      static const GtkTypeInfo image_info =
      {
	"GtkImageDB",
	sizeof (GtkImageDB),
	sizeof (GtkImageDBClass),
	(GtkClassInitFunc) gtk_imagedb_class_init,
	(GtkObjectInitFunc) gtk_imagedb_init,
	/* reserved_1 */ NULL,
        /* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      image_type = gtk_type_unique (GTK_TYPE_MISC, &image_info);
    }

  return image_type;
}

static void
gtk_imagedb_class_init (GtkImageDBClass *class)
{
  GtkWidgetClass *widget_class;

  widget_class = (GtkWidgetClass*) class;

  widget_class->expose_event = gtk_imagedb_expose;
}

static void
gtk_imagedb_init (GtkImageDB *image)
{
  GTK_WIDGET_SET_FLAGS (image, GTK_NO_WINDOW);

  image->image[0] = NULL;
  image->image[1] = NULL;
  
  image->current = 1;
  
  image->mask = NULL;
}

GtkWidget*
gtk_imagedb_new (GdkImage  *val1, GdkImage *val2,
	       GdkBitmap *mask)
{
  GtkImageDB *image;

  g_return_val_if_fail (val1 != NULL, NULL);
  g_return_val_if_fail (val2 != NULL, NULL);


  g_return_val_if_fail (val1->width == val2->width, NULL);
  g_return_val_if_fail (val1->height == val2->height, NULL);
  g_return_val_if_fail (val1->depth == val2->depth, NULL);
  g_return_val_if_fail (val1->bpl == val2->bpl, NULL);

  image = gtk_type_new (GTK_TYPE_IMAGEDB);

  gtk_imagedb_set (image, val1, val2, mask);

  return GTK_WIDGET (image);
}

void
gtk_imagedb_set (GtkImageDB  *image,
	       GdkImage  *val1, GdkImage *val2,
	       GdkBitmap *mask)
{
  g_return_if_fail (image != NULL);
  g_return_if_fail (GTK_IS_IMAGEDB (image));
  
  g_return_if_fail (val1 != NULL);
  g_return_if_fail (val2 != NULL);
  
  g_return_if_fail (val1->width == val2->width);
  g_return_if_fail (val1->height == val2->height);
  g_return_if_fail (val1->depth == val2->depth);
  g_return_if_fail (val1->bpl == val2->bpl);

  image->image[0] = val1;
  image->image[1] = val2;
  
  image->current = 1;
  
  image->mask = mask;

  GTK_WIDGET (image)->requisition.width = val1->width + GTK_MISC (image)->xpad * 2;
  GTK_WIDGET (image)->requisition.height = val1->height + GTK_MISC (image)->ypad * 2;

  if (GTK_WIDGET_VISIBLE (image))
	gtk_widget_queue_resize (GTK_WIDGET (image));
}

void
gtk_imagedb_get (GtkImageDB   *image,
	       GdkImage  **val,
	       GdkBitmap **mask)
{
  g_return_if_fail (image != NULL);
  g_return_if_fail (GTK_IS_IMAGEDB (image));

  if (val)
    *val = image->image[image->current];
  if (mask)
    *mask = image->mask;
}


static gint
gtk_imagedb_expose (GtkWidget      *widget,
		  GdkEventExpose *event)
{
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_IMAGEDB (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (GTK_WIDGET_VISIBLE (widget) && GTK_WIDGET_MAPPED (widget))
    {
      GtkImageDB *image;
      GtkMisc *misc;
      GdkRectangle area, image_bound, intersection;
      gint x, y;
      
      image = GTK_IMAGEDB (widget);
      misc = GTK_MISC (widget);

      x = (widget->allocation.x * (1.0 - misc->xalign) +
	   (widget->allocation.x + widget->allocation.width
	    - (widget->requisition.width - misc->xpad * 2)) *
	   misc->xalign) + 0.5;
      y = (widget->allocation.y * (1.0 - misc->yalign) +
	   (widget->allocation.y + widget->allocation.height
	    - (widget->requisition.height - misc->ypad * 2)) *
	   misc->yalign) + 0.5;

      if (image->mask)
	{
	  gdk_gc_set_clip_mask (widget->style->black_gc, image->mask);
	  gdk_gc_set_clip_origin (widget->style->black_gc, x, y);
	}

      image_bound.x = x;
      image_bound.y = y;      
      image_bound.width = image->image[image->current]->width;
      image_bound.height = image->image[image->current]->height;      

      area = event->area;
      
      if(gdk_rectangle_intersect(&image_bound, &area, &intersection))
        {
          gdk_draw_image (widget->window,
                          widget->style->black_gc,
                          image->image[image->current],
                          image_bound.x - x, image_bound.y - y,
                          image_bound.x, image_bound.y,
                          image_bound.width, image_bound.height);
        }
      
      if (image->mask)
	{
	  gdk_gc_set_clip_mask (widget->style->black_gc, NULL);
	  gdk_gc_set_clip_origin (widget->style->black_gc, 0, 0);
	}
    }

  return FALSE;
}

void gtk_imagedb_draw (GtkImageDB *image, GdkRectangle *rect)
{
  g_return_if_fail (image != NULL);
  g_return_if_fail (GTK_IS_IMAGEDB (image));

  if (GTK_WIDGET_VISIBLE (image) && GTK_WIDGET_MAPPED (image))
    {
	GtkWidget *widget = GTK_WIDGET(image);
	
	gint x, y, width, height;
      
      if (rect) {
      	x = rect->x;
      	y = rect->y;
      	width = rect->width;
      	height = rect->height;
      } else {
      		x = 0;
      		y = 0;
	      width = image->image[image->current]->width;
	      height = image->image[image->current]->height;
	}

          gdk_draw_image (widget->window,
                          widget->style->black_gc,
                          image->image[image->current],
                          0, 0,
                          x, y,
                          width, height);
    }
}

gint gtk_imagedb_swap(GtkImageDB *image)
{
  gint ret;
  
  g_return_val_if_fail (image != NULL, -1);
  g_return_val_if_fail (GTK_IS_IMAGEDB (image), -1);
  
  ret = image->current;
  
  image->current ^= 1;
  
  return ret;
}
