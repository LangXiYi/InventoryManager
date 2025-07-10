// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Components/InvSys_TagSlot.h"


TSharedRef<SWidget> UInvSys_TagSlot::RebuildWidget()
{
	MyBox = SNew(SBox);

	if ( IsDesignTime() )
	{
		MyBox->SetContent(
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromName(SlotTag.GetTagName()))
			]
		);
	}

	// Add any existing content to the new slate box
	if ( GetChildrenCount() > 0 )
	{
		UPanelSlot* ContentSlot = GetContentSlot();
		if ( ContentSlot->Content )
		{
			MyBox->SetContent(ContentSlot->Content->TakeWidget());
		}
	}

	return MyBox.ToSharedRef();
}
