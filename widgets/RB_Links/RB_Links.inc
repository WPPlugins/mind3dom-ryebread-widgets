<?php

if (!class_exists('RB_Link')):
	class RB_Link {
		private $lobj;
		
		public function __get($nm){
			$props = get_object_vars($this->lobj);
			return $props[$nm];
		}
		
		public function Link(){
			switch ($this->lobj->link_type) {
				case RB_Links::LT_JS:{
					return "javascript:"+$this->lobj->link;
				}break;
				case RB_Links::LT_URL:{
					if (preg_match('/^[\\\\\/].*/',$this->lobj->link))	
						return get_option("home").$this->lobj->link;
					else
						return $this->lobj->link;
				}break;
			}
			return "";
		}
		
		public function Icon(){
			if (preg_match('/^[\\\\\/].*/',$this->lobj->icon)){	

				switch ($lobj->icon_base_type){
					case RB_Links::BT_HOME:{
						return get_option("home").$this->lobj->icon;
					}break;
					case RB_Links::BT_TEMPLATE:{
						return get_template_directory_uri().$this->lobj->icon;
					}break;
				}
			} else 
				return $this->lobj->icon;
		}
		
		public function __construct($obj){
			$this->lobj = $obj;
		}
	}
endif;

if (!class_exists('RB_Links')):

class RB_Links extends RyeBread_Widget {
	private $links;
	public $Title;
	private $sort;
	
	const LT_URL = 0;
	const LT_JS = 1;
	
	const BT_TEMPLATE = 0;
	const BT_HOME = 1;
	
	const SORT_TITLE = 0;
	const SORT_ORDER = 1;
	
	public static $LinkTypes = array(
			self::LT_URL => "url",
			self::LT_JS => "script"
	);
		
	public static $IconBaseTypes = array(
		self::BT_HOME => "relative to home",
		self::BT_TEMPLATE => "relative to template"
	);
	
	public static $SortTypes = array(
		self::SORT_TITLE => "title", self::SORT_ORDER => "order setting"
	);

	public function __construct($id,$name,$skin){
		parent::__construct($id,$name,$skin);
		global $wpdb;
		$r = $wpdb->get_row("select * from ".$wpdb->prefix."rblinkswidgets where id=".$wpdb->escape($this->Id),OBJECT);
		$this->Title = $r->title;
		$this->sort = $r->sortlinks;
	}
	
	public function HandleAjax(){}
	
	public static function Install(){
		rb_register_widget_class("RB_Links");
		global $wpdb;
		$q = "
		create table ".$wpdb->prefix."rblinkswidgets (
		`id` int(5) not null,
		`ilc` bit default 0,
		`title` varchar(200),
		`sortlinks` int(1),
		primary key id (id),
		CONSTRAINT `rblw_fk1` FOREIGN KEY (id) REFERENCES ".$wpdb->prefix."rbwidgets (id) ON DELETE CASCADE
		) engine=InnoDB
		";
		$wpdb->query($q);
		$q = "
		create table ".$wpdb->prefix."rblwlinks (
		`id` bigint(20) not null AUTO_INCREMENT,
		`widget` int(5) not null,
		`title` text,
		`icon` varchar(500),
		`link` text,
		`link_type` int(1) not null default 0,
		`icon_base_type` int(1) not null default 0,
		`order` int(4) default 0, 
		primary key id (id),
		CONSTRAINT `rblwl_fk1` FOREIGN KEY (widget) REFERENCES ".$wpdb->prefix."rbwidgets (id) ON DELETE CASCADE
		) engine=InnoDB
		";
		$wpdb->query($q);
	}
	
	public static function UnInstall(){
		global $wpdb;
		rb_remove_widget_class("RB_Links");
		$wpdb->query("drop table ".$wpdb->prefix."rblwlinks");
		$wpdb->query("drop table ".$wpdb->prefix."rblinkswidgets");
	}
	
	public function ConfigPage(){
		global $wpdb;
		$r = $wpdb->get_row("select * from ".$wpdb->prefix."rblinkswidgets where id=".$wpdb->escape($this->Id),OBJECT);		
		?>
	<form id="rblwconf_<?php echo $this->Name; ?>" action="<?php echo get_option('siteurl')."/wp-admin/admin-ajax.php"; ?>" method="post" ajax="true" responsetype="script">
	 <input type="hidden" name="action" value="rb_admin_call" />
	 <input type="hidden" name="rbadminaction" value="configwidget" />
	 <input type="hidden" name="widget" value="<?php echo $this->Id; ?>" />	
  	<table>
  	<tr><td><?php _e("Title");?></td><td><input type="text" name="title" value="<?php echo $r->title; ?>" size="30"/></td></tr>
  	<tr><td><?php _e("Sort links by");?></td><td>
  	<select name="sort">
  		<?php
  			foreach (self::$SortTypes as $key=>$value){
  				?>
  				<option value="<?php echo $key; ?>" <?php echo ($key == $r->sortlinks)?"selected":""; ?>><?php _e($value); ?></option>
  				<?php
  			}
  		 ?>
  	</select>
  	</td></tr>
  	</table>
  	<?php
  	/* <tr><td><?'php _e("Use links category"); ?'></td><td><input type="checkbox" name="ilc" <?'php echo ($r->ilc == 1?'checked':''); ?'>/></td></tr> */ 
  	?>
  	<div style="text-align:right;"><button id="savebtn_<?php echo $this->Id; ?>" onclick="javascript:this.disabled = true;jQuery(&quot;#rblwconf_<?php echo $this->Name; ?>&quot;).submit();"><?php _e("Apply"); ?></button></div>
  	</form>
	<div class="rb_form_heading"><?php _e("Add Link");?></div>
	<form id="rbaddlnk_<?php echo $this->Name; ?>" action="<?php echo get_option('siteurl')."/wp-admin/admin-ajax.php"; ?>" method="post" ajax="true" callback="jQuery(&quot;#rblwlinks_<?php echo $this->Id; ?>&quot;).append(jQuery(data));">
	 <input type="hidden" name="action" value="rb_admin_call" />
	 <input type="hidden" name="rbadminaction" value="rblwaddlink" />
	 <input type="hidden" name="widget" value="<?php echo $this->Id; ?>" />	
	<div>
	<table>
	<tr><td><?php _e("Title");?></td><td><input type="text" name="title" value="" size="40"/></td></tr>
	<tr><td><?php _e("Icon");?></td><td><input type="text" name="icon" value="" size="40"/></td></tr>
	<tr><td><?php _e("Link");?></td><td><textarea name="link" rows="5" cols="40"><?php ?></textarea></td></tr>
	<tr><td><?php _e("Link type");?></td><td>
		<select name="linktype">
		<?php
			foreach (self::$LinkTypes as $key=>$value){
				?><option value="<?php echo $key; ?>"><?php echo _e($value); ?></option><?php
			}
		?>
		</select>
	</td></tr>
	<tr><td><?php _e("Icon base");?></td><td>
		<select name="iconbase">
		<?php
			foreach (self::$IconBaseTypes as $key=>$value){
				?><option value="<?php echo $key; ?>"><?php echo _e($value); ?></option><?php
			}
		?>
		</select>
	</td></tr>
	</table>
	<button onclick="jQuery(&quot;#rbaddlnk_<?php echo $this->Name; ?>&quot;).submit();"><?php _e("Add"); ?></button>
	</div>
	</form>
	<div class="rb_form_heading"><?php _e("Links list");?></div>
  	<div id="rblwlinks_<?php echo $this->Id; ?>">
	<?php
		$links = $wpdb->get_results("select * from ".$wpdb->prefix."rblwlinks where widget=".$wpdb->escape($this->Id),OBJECT);
		if (is_array($links))
		foreach ($links as $link)
			$this->OutLinkConfig($link);
	?>
	
  	</div>
		<?php
	}
	
	protected function OutLinkConfig($link){
?>
<div id="rblwl_<?php echo $link->id; ?>" class="rb_formfield">
	<div class="rb_link_title" onclick="javascript:jQuery(&quot;#rbwlp_<?php echo $link->id; ?>&quot;).css(&quot;display&quot;,(jQuery(&quot;#rbwlp_<?php echo $link->id; ?>&quot;).css(&quot;display&quot;) == &quot;none&quot;)?&quot;block&quot;:&quot;none&quot;);" >
		<table style="width:100%">
			<tr>
				<td>
				<?php echo $link->title; ?>&nbsp;:&nbsp;<?php echo self::$LinkTypes[$link->link_type]; ?>
				</td>
				<td style="text-align:right;">
					<button onclick="javascript:jQuery.ajax({data: {action:&quot;rb_admin_call&quot;,rbadminaction:&quot;rblwdellink&quot;,widget:&quot;<?php echo $this->Id; ?>&quot;,linkid:&quot;<?php echo $link->id; ?>&quot;},type:&quot;POST&quot;,dataType:&quot;script&quot;, url:&quot;<?php echo get_option('siteurl')."/wp-admin/admin-ajax.php";  ?>&quot;});"><?php _e("Delete"); ?></button>
				</td>
			</tr>
		</table>
	</div>
	<div id="rbwlp_<?php echo $link->id; ?>" class="rb_link_properties" style="display:none;">
  	<form id="rbwlpf_<?php echo $link->id; ?>" action="<?php echo get_option('siteurl')."/wp-admin/admin-ajax.php";  ?>" method="post" ajax="true" responsetype="script">
	 		<input type="hidden" name="action" value="rb_admin_call" />
	 		<input type="hidden" name="rbadminaction" value="rblweditlink" />
	 		<input type="hidden" name="widget" value="<?php echo $this->Id; ?>" />
	 		<input type="hidden" name="linkid" value="<?php echo $link->id; ?>" />	
	<table>
	<tr><td><?php _e("Title");?></td><td><input type="text" name="title" value="<?php echo $link->title; ?>" size="40"/></td></tr>
	<tr><td><?php _e("Icon");?></td><td><input type="text" name="icon" value="<?php echo $link->icon; ?>" size="40"/></td></tr>
	<tr><td><?php _e("Link");?></td><td><textarea name="link" rows="5" cols="40"><?php echo $link->link; ?></textarea></td></tr>
	<tr><td><?php _e("Link type");?></td><td>
		<select name="linktype">
		<?php
			foreach (self::$LinkTypes as $key=>$value){
				?><option value="<?php echo $key; ?>" <?php echo (($key == $link->link_type)?"selected":""); ?>><?php echo _e($value); ?></option><?php
			}
		?>
		</select>
	</td></tr>
	<tr><td><?php _e("Icon base");?></td><td>
		<select name="iconbase">
		<?php
			foreach (self::$IconBaseTypes as $key=>$value){
				?><option value="<?php echo $key; ?>" <?php echo ($key == $link->icon_base_type)?"selected":""; ?>><?php echo _e($value); ?></option><?php
			}
		?>
		</select>
	</td></tr>
	<tr><td><?php _e("Order");?></td><td><input type="text" name="order" size="4" value="<?php echo $link->order; ?>" /></td></tr>
	</table>
  			<button id="rbwleb_<?php echo $link->id; ?>" onclick="javascript:this.disabled=true;jQuery(&quot;#rbwlpf_<?php echo $link->id; ?>&quot;).submit();"><?php _e("Apply"); ?></button>
  		</form>
	</div>
</div>
		<?php		
	}	
	
	public function HandleConfigAction($action = null){
		global $wpdb;
		switch ($action){
			case "configwidget":{
				$is_ajax = ($_REQUEST["ilc"] == "on")?1:0;
				if ($r = $wpdb->get_row("select id from ".$wpdb->prefix."rblinkswidgets where id=".$wpdb->escape($this->Id),OBJECT))	
					$wpdb->update($wpdb->prefix."rblinkswidgets",array("title"=>$wpdb->escape($_REQUEST["title"]),"sortlinks"=>$wpdb->escape($_REQUEST["sort"])),array("id"=>$wpdb->escape($this->Id)));
				else
					$wpdb->insert($wpdb->prefix."rblinkswidgets",array("id"=>$wpdb->escape($this->Id),"title"=>$wpdb->escape($_REQUEST["title"]),"sortlinks"=>$wpdb->escape($_REQUEST["sort"])));
			?>
			document.getElementById("savebtn_<?php echo $_REQUEST["widget"]; ?>").disabled = false;
			<?php		
			}break;
			case "rblwaddlink":{
				$wpdb->insert($wpdb->prefix."rblwlinks",array("widget"=>$wpdb->escape($this->Id),"title"=>$wpdb->escape($_REQUEST["title"]),"icon"=>$wpdb->escape($_REQUEST["icon"]),"link"=>$wpdb->escape($_REQUEST["link"]),"link_type"=>$wpdb->escape($_REQUEST["linktype"]),"icon_base_type"=>$wpdb->escape($_REQUEST["iconbase"])));
				$id = $wpdb->insert_id;
				$link = $this->getLink($id);
				header("Cache-Control:no-cache");
				header("Pragma:no-cache");
				$this->OutLinkConfig($link);
			}break;
			case "rblwdellink":{
				$wpdb->query("delete from ".$wpdb->prefix."rblwlinks where id=".$wpdb->escape($_REQUEST["linkid"]));
				?>
				jQuery("#rblwl_<?php echo $_REQUEST["linkid"]; ?>").remove();
				<?php
			}break;
			case "rblweditlink":{
				$wpdb->update($wpdb->prefix."rblwlinks",array("title"=>$wpdb->escape($_REQUEST["title"]),"icon"=>$wpdb->escape($_REQUEST["icon"]),"link"=>$wpdb->escape($_REQUEST["link"]),"link_type"=>$wpdb->escape($_REQUEST["linktype"]),"icon_base_type"=>$wpdb->escape($_REQUEST["iconbase"]),"order"=>$wpdb->escape($_REQUEST["order"])),array("id"=>$wpdb->escape($_REQUEST["linkid"])));	
				?>
				jQuery("#rbwleb_<?php echo $_REQUEST["linkid"]; ?>").attr("disabled",false);
				<?php
			}break;
		}
	}
	
	protected function getLink($id){
		global $wpdb;
		$link = $wpdb->get_row("select * from ".$wpdb->prefix."rblwlinks where id=".$wpdb->escape($id),OBJECT);
		return new RB_Link($link);
	}
	
	protected function getLinks(){
		if (!isset($this->links)){
			$this->links = array();
			global $wpdb;
			$q = "select * from ".$wpdb->prefix."rblwlinks where widget=".$wpdb->escape($this->Id);
			switch ($this->sort){
				case self::SORT_ORDER: $q .= " order by `order`";break;
				case self::SORT_TITLE: $q .= " order by `title`";break;
			}
			$temp_links = $wpdb->get_results($q,OBJECT);
			foreach ($temp_links as &$link)
				$this->links[] = new RB_Link($link);
		}
		return $this->links;
	}
	

	protected function DefaultRendering(){
		$links = $this->getLinks();
		foreach ($links as $link){
		?>
		<a href="<?php echo $link->Link(); ?>" alt="<?php echo $link->title; ?>"><img src="<?php echo $link->Icon(); ?>" border="0" /></a>
		<?php
		}
	}
}

endif;

global $plugin_dir;
wp_register_style("rblinksadmincss",plugins_url()."/".$plugin_dir."/widgets/RB_Form/rblinks_admin.css");
add_action('admin_init', create_function('', 'wp_enqueue_style("rblinksadmincss");'));

//wp_register_script("jquery-effects-core",plugins_url()."/".$plugin_dir."/widgets/RB_Rollover/effects.core.js");
//wp_register_script("jquery-effects-scale",plugins_url()."/".$plugin_dir."/widgets/RB_Rollover/effects.scale.js");
//add_action('init', create_function('', 'wp_enqueue_script("jquery-effects-core");wp_enqueue_script("jquery-effects-scale");'));

?>
