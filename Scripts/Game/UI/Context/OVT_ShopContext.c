class OVT_ShopContext : OVT_UIContext
{	
	protected OVT_ShopComponent m_Shop;
	protected int m_iPageNum = 0;
	protected int m_SelectedResource = -1;
	protected ResourceName m_SelectedResourceName;
	protected int m_iNumPages = 0;
		
	override void PostInit()
	{
		if(SCR_Global.IsEditMode()) return;
		m_Economy.m_OnPlayerMoneyChanged.Insert(OnPlayerMoneyChanged);
	}
	
	override void RegisterInputs()
	{
		super.RegisterInputs();
		if(!m_InputManager) return;
		
		m_InputManager.AddActionListener("MenuNavLeft", EActionTrigger.DOWN, PreviousPage);		
		m_InputManager.AddActionListener("MenuNavRight", EActionTrigger.DOWN, NextPage);
	}
	
	override void UnregisterInputs()
	{
		super.UnregisterInputs();
		if(!m_InputManager) return;
		
		m_InputManager.RemoveActionListener("MenuNavLeft", EActionTrigger.DOWN, PreviousPage);
		m_InputManager.RemoveActionListener("MenuNavRight", EActionTrigger.PRESSED, NextPage);				
	}
	
	protected void OnPlayerMoneyChanged(string playerId, int amount)
	{
		if(playerId == m_sPlayerID && m_bIsActive)
		{
			TextWidget money = TextWidget.Cast(m_wRoot.FindAnyWidget("PlayerMoney"));		
			money.SetText("$" + amount);
		}
	}
	
	override void OnShow()
	{	
		m_iPageNum = 0;	
		
		Widget buyButton = m_wRoot.FindAnyWidget("BuyButton");
		ButtonActionComponent action = ButtonActionComponent.Cast(buyButton.FindHandler(ButtonActionComponent));
		
		action.GetOnAction().Insert(Buy);
		
		Widget sellButton = m_wRoot.FindAnyWidget("SellButton");
		if(m_Shop.m_ShopType == OVT_ShopType.SHOP_GUNDEALER || m_Shop.m_ShopType == OVT_ShopType.SHOP_VEHICLE)
		{
			sellButton.SetVisible(false);
		}else{
			ButtonActionComponent sellAction = ButtonActionComponent.Cast(sellButton.FindHandler(ButtonActionComponent));
		
			sellAction.GetOnAction().Insert(Sell);
		}
		
		Widget prevButton = m_wRoot.FindAnyWidget("PrevButton");
		SCR_NavigationButtonComponent btn = SCR_NavigationButtonComponent.Cast(prevButton.FindHandler(SCR_NavigationButtonComponent));
		
		btn.m_OnClicked.Insert(PreviousPage);
		
		Widget nextButton = m_wRoot.FindAnyWidget("NextButton");
		btn = SCR_NavigationButtonComponent.Cast(nextButton.FindHandler(SCR_NavigationButtonComponent));
		
		btn.m_OnClicked.Insert(NextPage);
		
		Widget closeButton = m_wRoot.FindAnyWidget("CloseButton");
		btn = SCR_NavigationButtonComponent.Cast(closeButton.FindHandler(SCR_NavigationButtonComponent));		
		btn.m_OnClicked.Insert(CloseLayout);
		
		Refresh();		
	}
	
	void PreviousPage()
	{
		if(!m_wRoot) return;
		m_iPageNum--;
		if(m_iPageNum < 0) m_iPageNum = 0;
		
		Refresh();
	}
	
	void NextPage()
	{
		if(!m_wRoot) return;
		m_iPageNum++;
		if(m_iPageNum > m_iNumPages-1) m_iPageNum = m_iNumPages-1;
		
		Refresh();
	}
	
	void Refresh()
	{
		if(!m_Shop) return;
		if(!m_wRoot) return;
		
		TextWidget money = TextWidget.Cast(m_wRoot.FindAnyWidget("PlayerMoney"));
		
		money.SetText("$" + m_Economy.GetPlayerMoney(m_sPlayerID));
		
		TextWidget pages = TextWidget.Cast(m_wRoot.FindAnyWidget("Pages"));
		
		Widget grid = m_wRoot.FindAnyWidget("BrowserGrid");
				
		int wi = 0;
		
		if(m_Shop.m_bProcurement)
		{
			array<ResourceName> vehicles();
			m_Economy.GetAllNonOccupyingFactionVehicles(vehicles, true);
			
			m_iNumPages = Math.Ceil(vehicles.Count() / 15);
			if(m_iPageNum >= m_iNumPages) m_iPageNum = 0;
			string pageNumText = (m_iPageNum + 1).ToString();
			
			pages.SetText(pageNumText + "/" + m_iNumPages);
						
			for(int i = m_iPageNum * 15; i < (m_iPageNum + 1) * 15 && i < vehicles.Count(); i++)
			{	
				ResourceName res = vehicles[i];
				int id = m_Economy.GetInventoryId(res);
				if(wi == 0 && m_SelectedResource == -1){
					SelectItem(res);
				}
				Widget w = grid.FindWidget("ShopMenu_Card" + wi);
				w.SetOpacity(1);
				OVT_ShopMenuCardComponent card = OVT_ShopMenuCardComponent.Cast(w.FindHandler(OVT_ShopMenuCardComponent));
				
				int buy = m_Economy.GetPrice(id);
								
				card.Init(res, buy, 100, this);
				
				wi++;
			}
		}else{
			m_iNumPages = Math.Ceil(m_Shop.m_aInventory.Count() / 15);
			if(m_iPageNum >= m_iNumPages) m_iPageNum = 0;
			string pageNumText = (m_iPageNum + 1).ToString();
			
			pages.SetText(pageNumText + "/" + m_iNumPages);
			
			//We only read m_Shop.m_aInventory because m_Shop.m_aInventoryItems is not replicated
			for(int i = m_iPageNum * 15; i < (m_iPageNum + 1) * 15 && i < m_Shop.m_aInventory.Count(); i++)
			{			
				int id = m_Shop.m_aInventory.GetKey(i);
				ResourceName res = m_Economy.GetResource(id);
							
				if(wi == 0 && m_SelectedResource == -1){
					SelectItem(res);
				}
				
				Widget w = grid.FindWidget("ShopMenu_Card" + wi);
				w.SetOpacity(1);
				OVT_ShopMenuCardComponent card = OVT_ShopMenuCardComponent.Cast(w.FindHandler(OVT_ShopMenuCardComponent));
				
				int buy = m_Economy.GetBuyPrice(id, m_Shop.GetOwner().GetOrigin(),m_iPlayerID);
				int qty = m_Shop.GetStock(id);
				
				card.Init(res, buy, qty, this);
				
				wi++;
			}
		}
		
		for(; wi < 15; wi++)
		{
			Widget w = grid.FindWidget("ShopMenu_Card" + wi);			
			w.SetOpacity(0);
		}
		
	}
	
	override void SelectItem(ResourceName res)
	{
		int id = m_Economy.GetInventoryId(res);
		m_SelectedResource = id;
		m_SelectedResourceName = res;
		TextWidget typeName = TextWidget.Cast(m_wRoot.FindAnyWidget("SelectedTypeName"));
		TextWidget details = TextWidget.Cast(m_wRoot.FindAnyWidget("SelectedDetails"));
		TextWidget desc = TextWidget.Cast(m_wRoot.FindAnyWidget("SelectedDescription"));
		
		int buy, sell, qty, max;
		
		if(m_Shop.m_bProcurement)
		{
			buy = m_Economy.GetPrice(id);
			sell = buy;
			qty = 100;
			max = 100;
		}else{
			buy = m_Economy.GetBuyPrice(id, m_Shop.GetOwner().GetOrigin(),m_iPlayerID);
			sell = m_Economy.GetSellPrice(id, m_Shop.GetOwner().GetOrigin());
			qty = m_Shop.GetStock(id);
			OVT_TownData town = m_Shop.GetTown();
			int townID = OVT_Global.GetTowns().GetTownID(town);
			max = m_Economy.GetTownMaxStock(townID, id);
		}	
		
		if(OVT_Global.ResourceIsVehicle(res))
		{
			SCR_EditableVehicleUIInfo info = OVT_Global.GetVehicleUIInfo(res);
			if(info)
			{
				typeName.SetText(info.GetName());
				if(m_Shop.m_bProcurement)
				{
					details.SetText("$" + buy);				
				}else{
					details.SetText("$" + buy + "\n" + qty + " #OVT-Shop_InStock");				
				}
				
				desc.SetText(info.GetDescription());
			}
		}else{
			UIInfo info = OVT_Global.GetItemUIInfo(res);
			if(info)
			{
				typeName.SetText(info.GetName());
				details.SetText("#OVT-Shop_Buying: $" + buy + "\n#OVT-Shop_Selling: $" + sell + "\n" + qty + "/" + max + " #OVT-Shop_InStock");
				desc.SetText(info.GetDescription());
			}
		}
	}
	
	void SetShop(OVT_ShopComponent shop)
	{
		m_Shop = shop;
	}		
	
	void Buy(Widget src, float value = 1, EActionTrigger reason = EActionTrigger.DOWN)
	{
		if(!m_Shop.m_bProcurement && m_Shop.GetStock(m_SelectedResource) < 1) return;
		
		int playerId = OVT_Global.GetPlayers().GetPlayerIDFromPersistentID(m_sPlayerID);
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if(!player) return;
		
		int cost = m_Economy.GetBuyPrice(m_SelectedResource, m_Shop.GetOwner().GetOrigin(),m_iPlayerID);
		
		if(m_Shop.m_bProcurement)
		{
			cost = m_Economy.GetPrice(m_SelectedResource);
		}
		
		if(!m_Economy.PlayerHasMoney(m_sPlayerID, cost)) return;
				
		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent( SCR_InventoryStorageManagerComponent ));
		if(!inventory) return;
		
		if(m_Shop.m_ShopType == OVT_ShopType.SHOP_VEHICLE)
		{
			OVT_Global.GetServer().BuyVehicle(m_Shop, m_SelectedResource, m_iPlayerID);	
			CloseLayout();
			return;
		}	
		
		OVT_Global.GetServer().Buy(m_Shop, m_SelectedResource, 1, m_iPlayerID);	
		SelectItem(m_SelectedResourceName);
	}
	
	void Sell(Widget src, float value = 1, EActionTrigger reason = EActionTrigger.DOWN)
	{
		int playerId = OVT_Global.GetPlayers().GetPlayerIDFromPersistentID(m_sPlayerID);
		IEntity player = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if(!player) return;
		
		int cost = m_Economy.GetSellPrice(m_SelectedResource, m_Shop.GetOwner().GetOrigin());
		
		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(player.FindComponent( SCR_InventoryStorageManagerComponent ));
		if(!inventory) return;
		
		array<IEntity> items = new array<IEntity>;
		inventory.GetItems(items);
		
		ResourceName res = m_Economy.GetResource(m_SelectedResource);
		
		foreach(IEntity ent : items)
		{
			if(ent.GetPrefabData().GetPrefabName() == res)
			{
				if(inventory.TryDeleteItem(ent))
				{
					m_Economy.AddPlayerMoney(m_iPlayerID, cost, true);
					m_Shop.AddToInventory(m_SelectedResource, 1);
					SelectItem(m_SelectedResourceName);
					break;
				}
			}
		}
	}
	
	void ~OVT_ShopContext()
	{
		if(!m_Economy) return;
		m_Economy.m_OnPlayerMoneyChanged.Remove(OnPlayerMoneyChanged);
	}
}