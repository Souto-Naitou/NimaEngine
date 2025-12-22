// ========================================
// NimaEngine リファレンスドキュメント - JavaScript
// ========================================

document.addEventListener('DOMContentLoaded', function() {
    initializeSearch();
    initializeSmoothScroll();
    initializeScrollSpy();
    initializeCodeHighlight();
    initializeMobileMenu();
    initializeHideHeaderOnScroll();
});

// ========================================
// 検索機能
// ========================================
function initializeSearch() {
    const searchInput = document.getElementById('searchInput');
    if (!searchInput) return;

    // 検索対象の要素を収集
    const searchableElements = collectSearchableElements();

    searchInput.addEventListener('input', function(e) {
        const query = e.target.value.toLowerCase().trim();
        
        if (query.length === 0) {
            clearSearchHighlights();
            showAllSections();
            return;
        }

        performSearch(query, searchableElements);
    });
}

function collectSearchableElements() {
    const elements = [];
    const sections = document.querySelectorAll('.section, .subsection');
    
    sections.forEach(section => {
        const text = section.textContent.toLowerCase();
        const id = section.id;
        const title = section.querySelector('h2, h3')?.textContent || '';
        
        elements.push({
            element: section,
            text: text,
            id: id,
            title: title
        });
    });
    
    return elements;
}

function performSearch(query, searchableElements) {
    let foundCount = 0;
    
    searchableElements.forEach(item => {
        const matches = item.text.includes(query);
        
        if (matches) {
            item.element.style.display = '';
            foundCount++;
            highlightText(item.element, query);
        } else {
            item.element.style.display = 'none';
        }
    });
    
    // 検索結果が0の場合
    if (foundCount === 0) {
        showNoResultsMessage();
    } else {
        removeNoResultsMessage();
    }
}

function highlightText(element, query) {
    // 既存のハイライトをクリア
    const textNodes = getTextNodes(element);
    
    textNodes.forEach(node => {
        const text = node.textContent;
        const lowerText = text.toLowerCase();
        const index = lowerText.indexOf(query);
        
        if (index !== -1) {
            const span = document.createElement('span');
            span.className = 'highlight';
            
            const before = text.substring(0, index);
            const match = text.substring(index, index + query.length);
            const after = text.substring(index + query.length);
            
            const parent = node.parentNode;
            parent.insertBefore(document.createTextNode(before), node);
            parent.insertBefore(span, node);
            span.textContent = match;
            parent.insertBefore(document.createTextNode(after), node);
            parent.removeChild(node);
        }
    });
}

function getTextNodes(element) {
    const textNodes = [];
    const walker = document.createTreeWalker(
        element,
        NodeFilter.SHOW_TEXT,
        {
            acceptNode: function(node) {
                // コードブロックやナビゲーション内は除外
                if (node.parentElement.closest('pre, code, nav')) {
                    return NodeFilter.FILTER_REJECT;
                }
                if (node.textContent.trim().length > 0) {
                    return NodeFilter.FILTER_ACCEPT;
                }
                return NodeFilter.FILTER_REJECT;
            }
        }
    );
    
    while (walker.nextNode()) {
        textNodes.push(walker.currentNode);
    }
    
    return textNodes;
}

function clearSearchHighlights() {
    const highlights = document.querySelectorAll('.highlight');
    highlights.forEach(highlight => {
        const text = highlight.textContent;
        const parent = highlight.parentNode;
        parent.replaceChild(document.createTextNode(text), highlight);
        parent.normalize();
    });
}

function showAllSections() {
    const sections = document.querySelectorAll('.section, .subsection');
    sections.forEach(section => {
        section.style.display = '';
    });
    removeNoResultsMessage();
}

function showNoResultsMessage() {
    removeNoResultsMessage();
    
    const content = document.querySelector('.content');
    const message = document.createElement('div');
    message.id = 'noResults';
    message.className = 'card text-center';
    message.innerHTML = `
        <h3>検索結果がありません</h3>
        <p class="text-muted">別のキーワードで検索してみてください。</p>
    `;
    content.insertBefore(message, content.firstChild);
}

function removeNoResultsMessage() {
    const message = document.getElementById('noResults');
    if (message) {
        message.remove();
    }
}

// ========================================
// スムーススクロール
// ========================================
function initializeSmoothScroll() {
    const links = document.querySelectorAll('a[href^="#"]');
    
    links.forEach(link => {
        link.addEventListener('click', function(e) {
            e.preventDefault();
            
            const targetId = this.getAttribute('href').substring(1);
            const targetElement = document.getElementById(targetId);
            
            if (targetElement) {
                targetElement.scrollIntoView({
                    behavior: 'smooth',
                    block: 'start'
                });
                
                // URLを更新
                history.pushState(null, null, `#${targetId}`);
                
                // ナビゲーションのアクティブ状態を更新
                updateActiveNavItem(this);
            }
        });
    });
}

// ========================================
// スクロールスパイ（ナビゲーションのアクティブ状態更新）
// ========================================
function initializeScrollSpy() {
    const sections = document.querySelectorAll('.section, .subsection');
    const navLinks = document.querySelectorAll('.sidebar a');
    
    window.addEventListener('scroll', throttle(function() {
        let currentSection = '';
        
        sections.forEach(section => {
            const sectionTop = section.offsetTop;
            const sectionHeight = section.clientHeight;
            
            if (window.pageYOffset >= sectionTop - 250) {
                currentSection = section.getAttribute('id');
            }
        });
        
        navLinks.forEach(link => {
            link.classList.remove('active');
            if (link.getAttribute('href') === `#${currentSection}`) {
                link.classList.add('active');
            }
        });
    }, 100));
}

function updateActiveNavItem(clickedLink) {
    const navLinks = document.querySelectorAll('.sidebar a');
    navLinks.forEach(link => link.classList.remove('active'));
    clickedLink.classList.add('active');
}

// ========================================
// コードブロックのハイライト（シンプル版）
// ========================================
function initializeCodeHighlight() {
    const codeBlocks = document.querySelectorAll('pre code');
    
    codeBlocks.forEach(block => {
        // コピーボタンを追加
        addCopyButton(block.parentElement);
    });
}

function addCopyButton(preElement) {
    const button = document.createElement('button');
    button.className = 'copy-btn';
    button.textContent = 'コピー';
    button.style.cssText = `
        position: absolute;
        top: 8px;
        right: 8px;
        padding: 4px 12px;
        background: rgba(255, 255, 255, 0.2);
        border: 1px solid rgba(255, 255, 255, 0.3);
        border-radius: 4px;
        color: white;
        cursor: pointer;
        font-size: 0.85rem;
        transition: all 0.2s ease;
    `;
    
    preElement.style.position = 'relative';
    
    button.addEventListener('click', function() {
        const code = preElement.querySelector('code').textContent;
        navigator.clipboard.writeText(code).then(() => {
            button.textContent = 'コピーしました！';
            setTimeout(() => {
                button.textContent = 'コピー';
            }, 2000);
        });
    });
    
    button.addEventListener('mouseenter', function() {
        button.style.background = 'rgba(255, 255, 255, 0.3)';
    });
    
    button.addEventListener('mouseleave', function() {
        button.style.background = 'rgba(255, 255, 255, 0.2)';
    });
    
    preElement.appendChild(button);
}

// ========================================
// モバイルメニュー
// ========================================
function initializeMobileMenu() {
    // モバイルメニュートグルボタンを作成
    const header = document.querySelector('.header');
    const sidebar = document.querySelector('.sidebar');
    
    if (window.innerWidth <= 768) {
        const toggleBtn = document.createElement('button');
        toggleBtn.className = 'mobile-menu-toggle';
        toggleBtn.innerHTML = '☰ 目次';
        toggleBtn.style.cssText = `
            position: fixed;
            bottom: 20px;
            right: 20px;
            padding: 12px 20px;
            background: var(--secondary-color);
            color: white;
            border: none;
            border-radius: 25px;
            font-size: 1rem;
            cursor: pointer;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
            z-index: 1000;
        `;
        
        toggleBtn.addEventListener('click', function() {
            sidebar.classList.toggle('mobile-active');
        });
        
        document.body.appendChild(toggleBtn);
    }
}

// ========================================
// ユーティリティ関数
// ========================================

// スロットル関数（パフォーマンス最適化）
function throttle(func, delay) {
    let lastCall = 0;
    return function(...args) {
        const now = new Date().getTime();
        if (now - lastCall < delay) {
            return;
        }
        lastCall = now;
        return func(...args);
    };
}

// デバウンス関数
function debounce(func, delay) {
    let timeoutId;
    return function(...args) {
        clearTimeout(timeoutId);
        timeoutId = setTimeout(() => func.apply(this, args), delay);
    };
}

// ========================================
// トップに戻るボタン
// ========================================
(function() {
    const scrollToTopBtn = document.createElement('button');
    scrollToTopBtn.innerHTML = '↑';
    scrollToTopBtn.className = 'scroll-to-top';
    scrollToTopBtn.style.cssText = `
        position: fixed;
        bottom: 80px;
        right: 20px;
        width: 50px;
        height: 50px;
        background: var(--accent-color);
        color: white;
        border: none;
        border-radius: 50%;
        font-size: 1.5rem;
        cursor: pointer;
        opacity: 0;
        visibility: hidden;
        transition: all 0.3s ease;
        box-shadow: 0 4px 8px rgba(0, 0, 0, 0.2);
        z-index: 999;
    `;
    
    document.body.appendChild(scrollToTopBtn);
    
    window.addEventListener('scroll', throttle(function() {
        if (window.pageYOffset > 300) {
            scrollToTopBtn.style.opacity = '1';
            scrollToTopBtn.style.visibility = 'visible';
        } else {
            scrollToTopBtn.style.opacity = '0';
            scrollToTopBtn.style.visibility = 'hidden';
        }
    }, 100));
    
    scrollToTopBtn.addEventListener('click', function() {
        window.scrollTo({
            top: 0,
            behavior: 'smooth'
        });
    });
    
    scrollToTopBtn.addEventListener('mouseenter', function() {
        this.style.transform = 'scale(1.1)';
    });
    
    scrollToTopBtn.addEventListener('mouseleave', function() {
        this.style.transform = 'scale(1)';
    });
})();

// ========================================
// ページ読み込み時の処理
// ========================================
window.addEventListener('load', function() {
    // URLのハッシュがある場合、その位置にスクロール
    if (window.location.hash) {
        setTimeout(() => {
            const target = document.querySelector(window.location.hash);
            if (target) {
                target.scrollIntoView({
                    behavior: 'smooth',
                    block: 'start'
                });
            }
        }, 100);
    }
    
    // フェードインアニメーション
    const cards = document.querySelectorAll('.card');
    cards.forEach((card, index) => {
        setTimeout(() => {
            card.style.animation = 'fadeIn 0.5s ease-in';
        }, index * 50);
    });
});

// ========================================
// キーボードショートカット
// ========================================
document.addEventListener('keydown', function(e) {
    // Ctrl + K または Cmd + K で検索フォーカス
    if ((e.ctrlKey || e.metaKey) && e.key === 'k') {
        e.preventDefault();
        const searchInput = document.getElementById('searchInput');
        if (searchInput) {
            searchInput.focus();
            searchInput.select();
        }
    }
    
    // ESC で検索クリア
    if (e.key === 'Escape') {
        const searchInput = document.getElementById('searchInput');
        if (searchInput && searchInput.value) {
            searchInput.value = '';
            searchInput.dispatchEvent(new Event('input'));
        }
    }
});

// ========================================
// コンソールメッセージ
// ========================================
console.log('%cNimaEngine リファレンスドキュメント', 'font-size: 20px; font-weight: bold; color: #3498db;');
console.log('%cキーボードショートカット:', 'font-size: 14px; font-weight: bold;');
console.log('Ctrl/Cmd + K: 検索フォーカス');

// ========================================
// スクロール時にヘッダーを隠す
// ========================================
function initializeHideHeaderOnScroll() {
    const header = document.querySelector('.header');
    if (!header) return;

    let lastScrollY = window.scrollY;
    let ticking = false;
    const threshold = 10; // 最小スクロール量

    function updateHeader() {
        const currentScrollY = window.scrollY;
        const scrollDiff = currentScrollY - lastScrollY;
        
        // スクロール差が閾値以上の場合のみ処理
        if (Math.abs(scrollDiff) > threshold) {
            if (currentScrollY > 100 && scrollDiff > 0) {
                // 下スクロール: ヘッダーを隠す
                header.classList.add('hidden');
            } else if (scrollDiff < 0) {
                // 上スクロール: ヘッダーを表示
                header.classList.remove('hidden');
            }
            lastScrollY = currentScrollY;
        }
        
        ticking = false;
    }

    window.addEventListener('scroll', function() {
        if (!ticking) {
            window.requestAnimationFrame(updateHeader);
            ticking = true;
        }
    }, { passive: true });
}
console.log('ESC: 検索クリア');