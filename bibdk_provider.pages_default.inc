<?php

/**
 * @file
 * This module provides default panels to demonstrate the behavior of the plugins.
 */

/**
 * Default panels pages for CTools Plugin Example
 *
 * To pick up this file, your module needs to implement
 * hook_ctools_plugin_api() - See ctools_plugin_example_ctools_plugin_api() in
 * ctools_plugin_example.module.
 *
 *
 * Note the naming of the file: <modulename>.pages_default.inc
 * With this naming, no additional code needs to be provided. CTools will just find the file.
 * The name of the hook is <modulename>_default_page_manager_pages()
 *
 * This example provides two pages, but the returned array could
 * have several pages.
 *
 * @return
 *   Array of pages, normally exported from Panels.
 */

function bibdk_provider_default_page_manager_pages() {
  // begin exported panel.
  $page = new stdClass;
  $page->disabled = FALSE; /* Edit this to true to make a default page disabled initially */
  $page->api_version = 1;
  $page->name = 'create_login';
  $page->task = 'page';
  $page->admin_title = 'Create login';
  $page->admin_description = 'create login for new user';
  $page->path = 'user/create_login';
  $page->access = array();
  $page->menu = array();
  $page->arguments = array();
  $page->conf = 
    array(
	  'admin_paths' => FALSE,
	  );
  $page->default_handlers = array();
  $handler = new stdClass;
  $handler->disabled = FALSE; /* Edit this to true to make a default handler disabled initially */
  $handler->api_version = 1;
  $handler->name = 'page_create_login_panel_context';
  $handler->task = 'page';
  $handler->subtask = 'create_login';
  $handler->handler = 'panel_context';
  $handler->weight = 0;
  $handler->conf = 
    array(
	  'title' => 'Panel',
	  'no_blocks' => 0,
	  'pipeline' => 'standard',
	  'css_id' => '',
	  'css' => '',
	  'contexts' => array(),
	  'relationships' => array(),
	  );
  $display = new panels_display;
  $display->layout = 'bibliomega_3col_11_9_4';
  $display->layout_settings = array();
  $display->panel_settings = 
    array(
	  'style_settings' => 
	  array(
		'default' => NULL,
		'left' => NULL,
		'middle' => NULL,
		'right' => NULL,
		),
	  );
  $display->cache = array();
  $display->title = '';
  $display->content = array();
  $display->panels = array();
  $pane = new stdClass;
  $pane->pid = 'new-1';
  $pane->panel = 'middle';
  $pane->type = 'bibdk_provider_pane';
  $pane->subtype = 'bibdk_provider_pane';
  $pane->shown = TRUE;
  $pane->access = array();
  $pane->configuration = array();
  $pane->cache = array();
  $pane->style = 
    array(
	  'settings' => NULL,
	  );
  $pane->css = array();
  $pane->extras = array();
  $pane->position = 0;
  $display->content['new-1'] = $pane;
  $display->panels['middle'][0] = 'new-1';
  $display->hide_title = PANELS_TITLE_FIXED;
  $display->title_pane = 'new-1';
  $handler->conf['display'] = $display;
  $page->default_handlers[$handler->name] = $handler;

  $pages['base_page'] = $page;

  // exported page

  $page = new stdClass;
  $page->disabled = FALSE; /* Edit this to true to make a default page disabled initially */
  $page->api_version = 1;
  $page->name = 'reset_password';
  $page->task = 'page';
  $page->admin_title = 'Reset password';
  $page->admin_description = 'user forgot password - reset';
  $page->path = 'user/password_reset';
  $page->access = array();
  $page->menu = array();
  // $page->arguments = array(0=>array('name'=>'HEST'));
  $page->arguments = array();
  $page->conf = array(
		      'admin_paths' => FALSE,
		      );
  $page->default_handlers = array();
  $handler = new stdClass;
  $handler->disabled = FALSE; /* Edit this to true to make a default handler disabled initially */
  $handler->api_version = 1;
  $handler->name = 'page_reset_password_panel_context';
  $handler->task = 'page';
  $handler->subtask = 'reset_password';
  $handler->handler = 'panel_context';
  $handler->weight = 0;
  $handler->conf = array(
			 'title' => 'Panel',
			 'no_blocks' => 0,
			 'pipeline' => 'standard',
			 'css_id' => '',
			 'css' => '',
			 'contexts' => array(),
			 'relationships' => array(),
			 );
  $display = new panels_display;
  $display->layout = 'bibliomega_3col_11_9_4';
  $display->layout_settings = array();
  $display->panel_settings = array(
				   'style_settings' => array(
							     'default' => NULL,
							     'left' => NULL,
							     'middle' => NULL,
							     'right' => NULL,
							     ),
				   );
  $display->cache = array();
  $display->title = '';
  $display->content = array();
  $display->panels = array();
  $pane = new stdClass;
  $pane->pid = 'new-1';
  $pane->panel = 'middle';
  $pane->type = 'bibdk_provider_user_reset';
  $pane->subtype = 'bibdk_provider_user_reset';
  $pane->shown = TRUE;
  $pane->access = array();
  $pane->configuration = array();
  $pane->cache = array();
  $pane->style = array(
		       'settings' => NULL,
		       );
  $pane->css = array();
  $pane->extras = array();
  $pane->position = 0;
  $display->content['new-1'] = $pane;
  $display->panels['middle'][0] = 'new-1';
  $display->hide_title = PANELS_TITLE_FIXED;
  $display->title_pane = '0';
  $handler->conf['display'] = $display;
  $page->default_handlers[$handler->name] = $handler;

  $pages['pass_page'] = $page;

  return $pages;  
}
