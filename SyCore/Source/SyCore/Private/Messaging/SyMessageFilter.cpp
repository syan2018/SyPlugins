#include "Messaging/SyMessageFilter.h"


bool USySourceTypeFilter::Matches(const FSyMessage& Message) const
{
    return Message.Source.SourceType == SourceType;
}

bool USySourceGuidFilter::Matches(const FSyMessage& Message) const
{
    return Message.Source.SourceId == SourceGuid;
}

bool USySourceAliasFilter::Matches(const FSyMessage& Message) const
{
    return Message.Source.SourceAlias == SourceAlias;
}

bool USyMessageTypeFilter::Matches(const FSyMessage& Message) const
{
    return Message.Content.MessageType == MessageType;
}

void USyMessageFilterComposer::AddFilter(USyMessageFilter* Filter)
{
    if (Filter)
    {
        Filters.Add(Filter);
    }
}

void USyMessageFilterComposer::ClearFilters()
{
    Filters.Empty();
}

bool USyMessageFilterComposer::Matches(const FSyMessage& Message) const
{
    for (const USyMessageFilter* Filter : Filters)
    {
        if (!Filter->Matches(Message))
        {
            return false;
        }
    }
    return true;
} 