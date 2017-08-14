<?php
if (!class_exists('RB_WPWidgetWrapper')):

class RB_WPWidgetWrapper extends RyeBread_Widget {
	protected $wp_widget;
	protected $widget_number;
	
	private static $wp_widgets;
	
	public function HandleAjax(){}
	
	public static function Install(){
		rb_register_widget_class("RB_WPWidgetWrapper");
		global $wpdb;

		$q = "
create table ".$wpdb->prefix."rbwpwwrapperwidgets (
`id` int(5) not null,
`wpwidget` varchar(200),
primary key id (id),
CONSTRAINT `rbwpwww_fk1` FOREIGN KEY (id) REFERENCES ".$wpdb->prefix."rbwidgets (id) ON DELETE CASCADE
) engine=InnoDB
";
		$wpdb->query($q);				
	}
	
	public static function UnInstall(){
		global $wpdb;
		$class = rb_installed_class(null,"RB_WPWidgetWrapper");
		$wpdb->query("delete from ".$wpdb->prefix."rbwidgets where classid=".$wpdb->escape($class->id));
		$wpdb->query("drop table ".$wpdb->prefix."rbwpwwrapperwidgets");
		rb_remove_widget_class("RB_WPWidgetWrapper");
	}
	
	public function __construct($id,$name,$skin=""){
		parent::__construct($id,$name,$skin);
		global $wpdb;
		$this->wp_widget = null;
		if ($r = $wpdb->get_row("select * from ".$wpdb->prefix."rbwpwwrapperwidgets where id=".$wpdb->escape($this->Id)))
		{
			$this->wp_widget = $r->wpwidget;
		}
	}
	
	protected static function getWpWidgets(){
		if (!is_array(self::$wp_widgets)){
			self::$wp_widgets = array();
			global $wp_registered_widgets, $wp_registered_widget_controls;
			foreach ( $wp_registered_widgets as $wpid => $widget )
				self::$wp_widgets[$widget["classname"]] = array("name"=>$widget["name"],"display"=>$widget["callback"],"control"=>$wp_registered_widget_controls[$widget["id"]]["callback"]);
		}
		return self::$wp_widgets;
	}
	
	protected function saveWpWidgetBtn(){
	?>
	<div style="text-align:right;"><button id="savewpwo_<?php echo $this->Id; ?>" onclick="javascript:this.disabled = true;jQuery(&quot;#wcf<?php echo $this->Id; ?>&quot;).submit();"><?php _e("Apply"); ?></button></div>
	<?php
	}
	
	public function ConfigPage(){
		global $wpdb;
		$sbid = register_sidebar();
		?>
	<form id="f<?php echo $this->Id; ?>" action="<?php echo get_option('siteurl')."/wp-admin/admin-ajax.php"; ?>" method="post" ajax="true" responsetype="script">
	 <input type="hidden" name="action" value="rb_admin_call" />
	 <input type="hidden" name="rbadminaction" value="configwidget" />
	 <input type="hidden" name="widget" value="<?php echo $this->Id; ?>" />	
  	<table>
  	<tr><td>WP widget</td><td>
  	<select name="wpwidget">
  	<?php
  		$wpw = self::getWpWidgets();
		foreach ( $wpw as $cn => $widget ){
			?><option value="<?php echo $cn; ?>" <?php echo ($cn == $this->wp_widget)?"selected":""; ?>><?php echo $widget["name"]; ?></option><?php 	
		}  	 
  	?>
  	</select></td></tr>
  	</table>
  	<div style="text-align:right;"><button id="savebtn_<?php echo $this->Id; ?>" onclick="javascript:this.disabled = true;jQuery(&quot;#f<?php echo $this->Id; ?>&quot;).submit();"><?php _e("Apply"); ?></button></div>
  	</form>
	<form id="wcf<?php echo $this->Id; ?>" action="<?php echo get_option('siteurl')."/wp-admin/admin-ajax.php"; ?>" method="post" ajax="true" responsetype="script">
	 <input type="hidden" name="action" value="rb_admin_call" />
	 <input type="hidden" name="rbadminaction" value="savewpwwrapperoptions" />
	 <input type="hidden" name="widget" value="<?php echo $this->Id; ?>" />
	 <input type="hidden" name="sidebar" value="<?php echo $sbid; ?>" />
	 <div id="wpwwrapper<?php echo $this->Id; ?>">
  	<?php 
  	if (isset($this->wp_widget)) {
  		$widget = $wpw[$this->wp_widget];
		if ($widget["control"]){
			$params = array("widget_args"=>$this->Id);  
			call_user_func_array( $widget["control"],  $params);
			$this->saveWpWidgetBtn();
		}
	 } ?>
	</div>  	
	</form>
		<?php		
	}
	
	public function HandleConfigAction($action = null){
			global $wpdb;
			
		switch ($action){
			case "configwidget":{
				$wpwidget = $_REQUEST["wpwidget"];	
				if ($r = $wpdb->get_row("select id from ".$wpdb->prefix."rbwpwwrapperwidgets where id=".$wpdb->escape($this->Id),OBJECT))	
					$wpdb->update($wpdb->prefix."rbwpwwrapperwidgets",array("wpwidget"=>$wpdb->escape($wpwidget)),array("id"=>$wpdb->escape($this->Id)));
				else
					$wpdb->insert($wpdb->prefix."rbwpwwrapperwidgets",array("id"=>$wpdb->escape($this->Id),"wpwidget"=>$wpdb->escape($wpwidget)));
			?>
			document.getElementById("savebtn_<?php echo $this->Id; ?>").disabled = false;
			<?php		
				$this->wp_widget = $wpwidget;
				$wpw = self::getWpWidgets();
				$widget  = $wpw[$this->wp_widget];
				if ($widget["control"]){
					ob_start();  
					$args = array("widget_args"=>$this->Id);
					call_user_func_array( $widget["control"],  $args);
					$this->saveWpWidgetBtn();
					$contents = preg_replace(array('/"/',"/\n/","/\r/"),array('\\"',"\\n","\\r"),ob_get_clean());
					?>
					jQuery("#wpwwrapper<?php echo $this->Id;  ?>").html("<?php echo $contents; ?>");
					<?php			
				}
			}break;
			case "savewpwwrapperoptions":{
				$wpw = self::getWpWidgets();
				$widget  = $wpw[$this->wp_widget];
				if ($widget["control"]){
					ob_start();  
					$args = array("widget_args"=>$this->Id);
					call_user_func_array( $widget["control"],  $args);
					ob_end_clean();			
				}
			?>
			document.getElementById("savewpwo_<?php echo $_REQUEST["widget"]; ?>").disabled = false;
			<?php		
			}break;
		}
	}

	protected function DefaultRendering(){
		$sbid = register_sidebar();
		$wpw = self::getWpWidgets();
		$widget  = $wpw[$this->wp_widget];
		$params = array("params"=>array(),"widget_args"=>$this->Id);
		if ( is_callable($widget["display"]) ) 
			call_user_func_array($widget["display"], $params);
	}
}

endif;

?>