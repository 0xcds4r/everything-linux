#pragma once

class MainSearchInterface : public Interface
{
public:
    void Display() override;
    void Render() override;
public:
    void RenderWindow();
    void RenderSearchInput();
    void HandleSearchQuery();
    void RenderSearchOptions();
    void RenderSearchResults();
    void HandleScrollPaging(int);
    void RenderPagination();
    void RenderFileActions();
    void UpdateDirectories();
    void HandleSelected();
public:
    void Toggle(bool state = false) { m_bMSAllowed = state; };

    bool IsNeverIndexing() {
        return m_cIndexingState == 3;
    }

    bool IsNowIndexing() {
        return m_cIndexingState == 1;
    }

    bool IsOverIndexing() {
        return m_cIndexingState == 0;
    }

private:
    bool m_bMSAllowed;
    uint8_t m_cIndexingState = 3;
};