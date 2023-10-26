// Fill out your copyright notice in the Description page of Project Settings.


#include "CoACDNotification.h"

// Copyright Epic Games, Inc. All Rights Reserved.
#include "Widgets/Notifications/GlobalNotification.h"
#include "Widgets/Notifications/SNotificationList.h"

/** Pointer to global status object */
FCoACDNotificationState* GCoACDNotificationState = NULL;

/** Notification class for asynchronous convex decomposition. */
class FCoACDNotificationImpl : public FGlobalNotification, public FCoACDNotificationState, public FTickableEditorObject
{
public:
	FCoACDNotificationImpl(void)
	{
		GCoACDNotificationState = this;	// Initialize the pointer to the global state notification
	}
	~FCoACDNotificationImpl(void)
	{
		GCoACDNotificationState = nullptr; // Clear the pointer to the global state notification
	}
protected:
	/** FGlobalEditorNotification interface */
	virtual bool ShouldShowNotification(const bool bIsNotificationAlreadyActive) const override;
	virtual void SetNotificationText(const TSharedPtr<SNotificationItem>& InNotificationItem) const override;
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual TStatId GetStatId() const override;
};

/** Global notification object. */
FCoACDNotificationImpl GCoACDNotification;

void FCoACDNotificationImpl::Tick(float DeltaTime)
{
	TickNotification(DeltaTime);
}

TStatId FCoACDNotificationImpl::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FGlobalEditorNotification, STATGROUP_Tickables);
}


bool FCoACDNotificationImpl::ShouldShowNotification(const bool bIsNotificationAlreadyActive) const
{
	return IsActive;
}

void FCoACDNotificationImpl::SetNotificationText(const TSharedPtr<SNotificationItem>& InNotificationItem) const
{
	if (IsActive)
	{
		FText ProgressMessage = FText::AsCultureInvariant(Status); // Text from V-HACD status is no localized
		InNotificationItem->SetText(ProgressMessage);
	}
}

