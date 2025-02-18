class OVT_PortItemComponent : SCR_ScriptedWidgetComponent
{
	protected ResourceName m_Resource;
	protected OVT_UIContext m_Context;
	
	void Init(ResourceName res, int cost, OVT_UIContext context)
	{	
		OVT_EconomyManagerComponent economy = OVT_Global.GetEconomy();	
				
		m_Resource = res;
		m_Context = context;
		
		OVT_OverthrowConfigComponent config = OVT_Global.GetConfig();
		
		TextWidget text = TextWidget.Cast(m_wRoot.FindAnyWidget("EntityName"));
		
		
		TextWidget qtyWidget = TextWidget.Cast(m_wRoot.FindAnyWidget("Cost"));
		
		qtyWidget.SetText("$" + cost.ToString());
		
		ItemPreviewWidget img = ItemPreviewWidget.Cast(m_wRoot.FindAnyWidget("Image"));
						
		ItemPreviewManagerEntity manager = GetGame().GetItemPreviewManager();
		if (!manager)
			return;
		
		// Set rendering and preview properties 
		
		img.SetResolutionScale(1, 1);
		manager.SetPreviewItemFromPrefab(img, res);		
		
		UIInfo info = OVT_Global.GetItemUIInfo(res);
		if(info)
		{
			text.SetText(info.GetName());
		}	
	}
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if(!m_Context) return false;
		
		super.OnClick(w, x, y, button);
		if (button != 0)
			return false;
		
		//m_Context.StartPlace(m_Placeable);
		m_Context.SelectItem(m_Resource);
		
		return true;
	}
}